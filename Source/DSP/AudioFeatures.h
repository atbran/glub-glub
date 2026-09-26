#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "TempoTracker.h"

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

    // Three-band levels (0..1, perceptual dB scale) and a bass-only kick pulse.
    float getLow() const noexcept { return bandLevel[0]; }
    float getMid() const noexcept { return bandLevel[1]; }
    float getHigh() const noexcept { return bandLevel[2]; }
    float getKick() const noexcept { return kick; }
    const TempoTracker& getTempo() const noexcept { return tempo; }

private:
    void finishFrame();

    double sampleRate = 44100.0;
    float energy = 0.0f;      // smoothed RMS 0..1
    float brightness = 0.0f;  // 0..1
    float beatPulse = 0.0f;   // onset pulse 0..1, decays
    float previousMagnitude = 0.0f;
    float fluxThreshold = 0.02f;
    float prevSample = 0.0f;

    // Band split: two one-pole lowpasses give low (<150 Hz), mid, high (>2.5 kHz).
    float lowCoeff = 0.0f, midCoeff = 0.0f;
    float lowState = 0.0f, midState = 0.0f;
    int hopSize = 441, hopFill = 0;
    double hopEnergy[3] {};
    float bandLevel[3] {};
    float bandLog[3] {};
    float kick = 0.0f, kickThreshold = 0.05f;
    TempoTracker tempo;
};
