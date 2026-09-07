#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

// Lightweight, allocation-free audio feature extractor.
// Call pushBlock() on the audio thread each processBlock.
class AudioFeatures
{
public:
    AudioFeatures();
    void prepare(double sampleRate);
    void pushBlock(const juce::AudioBuffer<float>& buffer);

    float getEnergy() const noexcept { return energy; }
    float getBrightness() const noexcept { return brightness; }
    float getBeatPulse() const noexcept { return beatPulse; }
    int getIntensity() const noexcept; // 0 Low, 1 Med, 2 High

private:
    double sampleRate = 44100.0;
    float energy = 0.0f;      // smoothed RMS 0..1
    float brightness = 0.0f;  // 0..1
    float beatPulse = 0.0f;   // onset pulse 0..1, decays
    float prevFlux = 0.0f;
    float fluxThreshold = 0.02f;
    float prevSample = 0.0f;
};
