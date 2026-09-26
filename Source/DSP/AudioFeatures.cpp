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

    const double twoPi = juce::MathConstants<double>::twoPi;
    lowCoeff = (float) (1.0 - std::exp(-twoPi * 150.0 / sampleRate));
    midCoeff = (float) (1.0 - std::exp(-twoPi * 2500.0 / sampleRate));
    lowState = midState = 0.0f;
    // ~100 analysis frames per second regardless of sample rate.
    hopSize = juce::jmax(16, (int) std::lround(sampleRate / 100.0));
    hopFill = 0;
    for (int b = 0; b < 3; ++b) { hopEnergy[b] = 0.0; bandLevel[b] = 0.0f; bandLog[b] = 0.0f; }
    kick = 0.0f;
    kickThreshold = 0.05f;
    tempo.prepare(sampleRate / hopSize);
}

void AudioFeatures::finishFrame()
{
    // Perceptual band levels: -54 dBFS maps to 0, -12 dBFS to 1. The mid and
    // high bands carry less power in typical mixes, so they get a small lift.
    static constexpr float gainDb[3] = { 0.0f, 4.0f, 10.0f };
    const float frameSeconds = (float) hopSize / (float) sampleRate;
    float onset = 0.0f;
    static constexpr float onsetWeight[3] = { 1.0f, 0.7f, 0.35f };
    for (int b = 0; b < 3; ++b)
    {
        const float rms = (float) std::sqrt(hopEnergy[b] / hopSize);
        const float db = 20.0f * std::log10(rms + 1.0e-7f) + gainDb[b];
        const float target = juce::jlimit(0.0f, 1.0f, (db + 54.0f) / 42.0f);
        const float tau = target > bandLevel[b] ? 0.025f : 0.22f;
        bandLevel[b] += (target - bandLevel[b]) * (1.0f - std::exp(-frameSeconds / tau));

        // Log-compressed positive flux per band is the onset strength.
        const float logEnergy = std::log1p(1000.0f * (float) (hopEnergy[b] / hopSize));
        onset += onsetWeight[b] * juce::jmax(0.0f, logEnergy - bandLog[b]);
        if (b == 0)
        {
            const float lowFlux = juce::jmax(0.0f, logEnergy - bandLog[0]);
            kickThreshold = 0.97f * kickThreshold + 0.03f * lowFlux + 0.002f;
            if (lowFlux > kickThreshold * 2.2f && lowFlux > 0.08f)
                kick = 1.0f;
        }
        bandLog[b] = logEnergy;
        hopEnergy[b] = 0.0;
    }
    kick *= std::exp(-frameSeconds / 0.12f);
    tempo.pushFrame(onset);
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

        lowState += lowCoeff * (mono - lowState);
        midState += midCoeff * (mono - midState);
        const float bands[3] = { lowState, midState - lowState, mono - midState };
        for (int b = 0; b < 3; ++b) hopEnergy[b] += (double) bands[b] * bands[b];
        if (++hopFill >= hopSize) { hopFill = 0; finishFrame(); }

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
