#include "AudioFeatures.h"
#include <cmath>

AudioFeatures::AudioFeatures() = default;

void AudioFeatures::prepare(double sr)
{
    sampleRate = sr > 0 ? sr : 44100.0;
    energy = 0.0f;
    brightness = 0.0f;
    beatPulse = 0.0f;
    previousMagnitude = 0.0f;
    fluxThreshold = 0.02f;
    prevSample = 0.0f;
}

int AudioFeatures::getIntensity() const noexcept
{
    if (energy > 0.45f || beatPulse > 0.7f) return 2;
    if (energy > 0.15f || beatPulse > 0.35f) return 1;
    return 0;
}

void AudioFeatures::pushBlock(const juce::AudioBuffer<float>& buffer)
{
    const int numCh = buffer.getNumChannels();
    const int n = buffer.getNumSamples();
    if (n <= 0 || numCh <= 0) return;

    double sumSq = 0.0;
    int zc = 0;

    for (int i = 0; i < n; ++i)
    {
        float mono = 0.0f;
        for (int c = 0; c < numCh; ++c)
            mono += buffer.getReadPointer(c)[i];
        mono /= (float) numCh;

        sumSq += (double) mono * mono;

        // zero-crossing (brightness proxy)
        if ((prevSample >= 0.0f) != (mono >= 0.0f)) zc++;
        prevSample = mono;
    }

    float flux = 0.0f;

    // proper flux: mean positive change of magnitude
    {
        double posSum = 0.0;
        for (int i = 0; i < n; i += 4)
        {
            float mono = 0.0f;
            for (int c = 0; c < numCh; ++c)
                mono += buffer.getReadPointer(c)[i];
            mono /= (float) numCh;
            float mag = std::abs(mono);
            float diff = mag - previousMagnitude;
            if (diff > 0) posSum += diff;
            previousMagnitude = mag;
        }
        flux = (float) (posSum / (n / 4 + 1) * 8.0);
    }

    float rms = (float) std::sqrt(sumSq / (n * 1.0));
    float targetEnergy = juce::jlimit(0.0f, 1.0f, rms * 2.2f);

    // envelope follower: fast attack, slow release
    const float blockSeconds = (float) (n / sampleRate);
    float attack = 1.0f - std::exp(-blockSeconds / 0.03f);
    float release = 1.0f - std::exp(-blockSeconds / 0.18f);
    float coeff = targetEnergy > energy ? attack : release;
    energy += coeff * (targetEnergy - energy);

    // onset pulse with adaptive threshold
    fluxThreshold = 0.985f * fluxThreshold + 0.015f * flux + 0.0004f;
    if (flux > fluxThreshold * 1.6f)
        beatPulse = juce::jmin(1.0f, beatPulse + flux * 4.0f);
    beatPulse *= std::exp(-blockSeconds / 0.11f);
    beatPulse = juce::jlimit(0.0f, 1.0f, beatPulse);

    // brightness: normalize zero-cross rate (expect ~0.02..0.25)
    float zcr = (float) zc / (float) n;
    float targetBright = juce::jlimit(0.0f, 1.0f, (zcr - 0.02f) * 5.0f);
    brightness += 0.15f * (targetBright - brightness);
}
