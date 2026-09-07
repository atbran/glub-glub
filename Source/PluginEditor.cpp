#include "PluginEditor.h"

GlubGlubEditor::GlubGlubEditor(GlubGlubProcessor& p)
    : AudioProcessorEditor(p), proc(p), drawer(p.apvts)
{
    setSize(500, 500);
    setResizable(true, true);
    setResizeLimits(320, 320, 1024, 1024);
    addAndMakeVisible(fish);
    addAndMakeVisible(bubbles);
    addAndMakeVisible(speech);
    addAndMakeVisible(drawer);
    addAndMakeVisible(hype);
    addAndMakeVisible(disco);
    disco.setInterceptsMouseClicks(false, false);
    drawer.onHeightChanged = [this] { resized(); };
    bubbles.setInterceptsMouseClicks(false, false);
    speech.setInterceptsMouseClicks(false, false);
    startTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    startTimerHz(60);
}

GlubGlubEditor::~GlubGlubEditor() { stopTimer(); }

void GlubGlubEditor::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    float hueShift = proc.apvts.getRawParameterValue("hue")->load();
    juce::Colour top = juce::Colour::fromHSV(0.58f + hueShift, 0.55f, 0.35f, 1.0f);
    juce::Colour bottom = juce::Colour::fromHSV(0.60f + hueShift, 0.6f, 0.12f, 1.0f);
    g.setGradientFill(juce::ColourGradient(top, 0, 0, bottom, 0, b.getHeight(), false));
    g.fillAll();
    // pebbles
    g.setColour(juce::Colour(0xFF2A3B4D));
    for (int i = 0; i < 8; ++i)
    {
        float x = std::fmod(i * 173.0f, b.getWidth());
        g.fillEllipse(x, b.getHeight() - 18 - (i % 3) * 8, 26, 12);
    }
    // seaweed
    g.setColour(juce::Colour(0xFF2E8B57));
    for (int i = 0; i < 3; ++i)
    {
        float x = 30 + i * 40;
        g.fillRect(x, b.getHeight() - 90.0f, 8.0f, 70.0f);
    }

    if (partyGlow > 0.004f)
    {
        float beatPulse = proc.vibe.beatPulse.load();
        float a = juce::jlimit(0.0f, 0.14f, (0.08f + 0.05f * beatPulse) * partyGlow);
        g.setColour(juce::Colour::fromHSV(partyHue, 0.55f, 0.9f, a));
        g.fillAll();
    }
}

void GlubGlubEditor::resized()
{
    auto b = getLocalBounds();
    drawer.setBounds(b.removeFromBottom(drawer.getCurrentHeight()));
    speech.setBounds(b.removeFromBottom(84));
    hype.setBounds(getWidth() - 182, 10, 170, 18);
    int discoW = juce::jlimit(120, 220, (int) (getWidth() * 0.32f));
    int discoH = (int) (discoW * 0.8f);
    disco.setBounds((int) (getWidth() * 0.5f) - discoW / 2, 0, discoW, discoH);
    fish.setBounds(b);
    bubbles.setBounds(b);
}

void GlubGlubEditor::timerCallback()
{
    float energy = proc.vibe.energy.load();
    float bright = proc.vibe.brightness.load();
    float pulse = proc.vibe.beatPulse.load();
    float phase = proc.vibe.beatPhase.load();
    int inten = proc.vibe.intensity.load();
    int bar = proc.vibe.barCount.load();
    bool bubblesOn = proc.apvts.getRawParameterValue("bubblesOn")->load() > 0.5f;
    float speechRate = proc.apvts.getRawParameterValue("speechRate")->load();
    double now = juce::Time::getMillisecondCounterHiRes() / 1000.0 - startTime;

    fish.setVibe(energy, bright, pulse, phase, inten, bar);

    float hypeLevel = juce::jlimit(0.0f, 1.0f, (energy * 0.5f + pulse * 0.3f + (float) inten * 0.1f) * 1.15f);
    hype.setHype(hypeLevel);
    disco.update(hypeLevel, now);

    if (phase >= 0.0f)
    {
        if (lastPhase >= 0.0f && phase < lastPhase - 0.5f)
            partyHue = std::fmod(partyHue + 0.27f, 1.0f);
    }
    else
    {
        partyHue = std::fmod(partyHue + 0.016f * (0.05f + pulse * 0.3f), 1.0f);
    }
    lastPhase = phase;

    float glowTarget = hypeLevel > 0.70f ? (hypeLevel - 0.70f) / 0.30f : 0.0f;
    partyGlow += (glowTarget - partyGlow) * 0.08f;
    if (partyGlow > 0.004f || lastGlow > 0.004f)
        repaint();
    lastGlow = partyGlow;

    bubbles.setEnabled(bubblesOn);
    if (bubblesOn && inten == 2 && pulse > 0.75f)
        bubbles.burst(fish.getMouthPosition());
    bubbles.update(energy, fish.getMouthPosition());
    speech.update(now, energy, inten, speechRate, rng);
}
