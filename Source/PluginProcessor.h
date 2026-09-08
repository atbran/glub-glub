#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include "DSP/AudioFeatures.h"
#include "DSP/VibeState.h"

class GlubGlubProcessor : public juce::AudioProcessor
{
public:
    GlubGlubProcessor();
    ~GlubGlubProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Glub-Glub"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    VibeState vibe;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParams();

private:
    AudioFeatures features;
    double smoothedBpm = 0.0;
    double lastPpq = 0.0;

#if GLUB_DEMO_MODE
    std::vector<float> demoBuf;
    juce::Random demoRandom;
    double demoPhase = 0.0;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlubGlubProcessor)
};
