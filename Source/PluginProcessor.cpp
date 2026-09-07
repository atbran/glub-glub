#include "PluginProcessor.h"
#include "PluginEditor.h"

GlubGlubProcessor::GlubGlubProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "GlubGlub", createParams())
{
}

GlubGlubProcessor::~GlubGlubProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout GlubGlubProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "speechRate", "Speech Rate", juce::NormalisableRange<float>(30.0f, 90.0f, 1.0f), 60.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "sensitivity", "Vibe Sensitivity", juce::NormalisableRange<float>(0.2f, 2.0f, 0.01f), 1.0f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        "hue", "Tank Hue", juce::NormalisableRange<float>(-0.1f, 0.1f, 0.001f), 0.0f));
    p.push_back(std::make_unique<juce::AudioParameterBool>("bubblesOn", "Bubbles", true));
    return { p.begin(), p.end() };
}

void GlubGlubProcessor::prepareToPlay(double sampleRate, int)
{
    features.prepare(sampleRate);
    smoothedBpm = 0.0;
}

void GlubGlubProcessor::releaseResources() {}

void GlubGlubProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Totally transparent: never touch audio samples.
    const int totalIn = getTotalNumInputChannels();
    const int totalOut = getTotalNumOutputChannels();
    for (int i = totalIn; i < totalOut; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float sensitivity = apvts.getRawParameterValue("sensitivity")->load();
    sensitivity = juce::jlimit(0.2f, 2.0f, sensitivity);

    features.pushBlock(buffer);

    float energy = juce::jlimit(0.0f, 1.0f, features.getEnergy() * sensitivity);
    float bright = features.getBrightness();
    float pulse = juce::jlimit(0.0f, 1.0f, features.getBeatPulse() * (0.5f + sensitivity * 0.5f));
    int intensity = features.getIntensity();
    if (sensitivity > 1.3f && energy > 0.3f && intensity < 2) intensity = 2;
    if (sensitivity < 0.7f && intensity > 0 && energy < 0.35f) intensity = 0;

    vibe.energy.store(energy);
    vibe.brightness.store(bright);
    vibe.beatPulse.store(pulse);
    vibe.intensity.store(intensity);
    vibe.barCount.store(-1);

    // Host BPM / beat phase (Ableton) — smoothed, fallback -1 when unknown.
    float beatPhase = -1.0f;
    double bpmToStore = 0.0;
    if (auto* playHead = getPlayHead())
    {
        if (auto pos = playHead->getPosition())
        {
            auto bpm = pos->getBpm();
            auto ppq = pos->getPpqPosition();
            if (bpm.hasValue())
            {
                double target = *bpm;
                if (smoothedBpm <= 0.0) smoothedBpm = target;
                else smoothedBpm += 0.2 * (target - smoothedBpm); // ~200ms smoothing
                bpmToStore = smoothedBpm;
            }
            if (ppq.hasValue() && bpmToStore > 0.0)
            {
                double frac = *ppq - std::floor(*ppq);
                if (frac < 0) frac += 1.0;
                beatPhase = (float) frac;
                int bar = (int) std::floor(*ppq / 4.0);
                vibe.barCount.store(bar);
            }
            lastPpq = ppq.hasValue() ? *ppq : lastPpq;
        }
    }
    vibe.beatPhase.store(beatPhase);
    vibe.bpm.store((float) bpmToStore);
}

juce::AudioProcessorEditor* GlubGlubProcessor::createEditor()
{
    return new GlubGlubEditor(*this);
}

void GlubGlubProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void GlubGlubProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GlubGlubProcessor();
}
