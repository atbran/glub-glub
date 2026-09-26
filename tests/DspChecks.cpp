#include "../Source/DSP/AudioFeatures.h"
#include <cmath>
#include <iostream>
#include <vector>

namespace
{
    bool passed = true;

    void check(bool ok, const std::string& label)
    {
        std::cout << label << ": " << (ok ? "PASS" : "FAIL") << '\n';
        passed &= ok;
    }

    // Kick on every beat, snare on 2 and 4, closed hats on the off-beat eighths.
    struct Groove
    {
        double bpm = 128.0, sr = 48000.0;
        bool snare = true, hats = true;
        juce::Random rng { 7 };
        long long n = 0;

        float next()
        {
            const double t = n++ / sr;
            const double beatLen = 60.0 / bpm;
            const double beats = t / beatLen;
            const double inBeat = (beats - std::floor(beats)) * beatLen;
            const int beatIndex = (int) std::floor(beats) % 4;
            const double pitch = 50.0 + 90.0 * std::exp(-inBeat / 0.03);
            float v = 0.85f * (float) (std::exp(-inBeat / 0.09) * std::sin(juce::MathConstants<double>::twoPi * pitch * inBeat));
            const float noise = rng.nextFloat() * 2.0f - 1.0f;
            if (snare && (beatIndex == 1 || beatIndex == 3))
                v += 0.35f * noise * (float) std::exp(-inBeat / 0.07);
            const double inEighth = std::fmod(inBeat, beatLen * 0.5);
            if (hats && inBeat >= beatLen * 0.5)
                v += 0.18f * noise * (float) std::exp(-inEighth / 0.015);
            v += 0.08f * (float) std::sin(juce::MathConstants<double>::twoPi * 110.0 * t); // pad
            return v;
        }

        double phaseAt() const
        {
            const double beats = (n / sr) * bpm / 60.0;
            return beats - std::floor(beats);
        }
    };

    template <typename Source>
    void run(AudioFeatures& features, Source&& source, double seconds, double sr, int block = 512)
    {
        std::vector<float> data((size_t) block);
        const int blocks = (int) (seconds * sr / block);
        for (int b = 0; b < blocks; ++b)
        {
            for (auto& s : data) s = source();
            float* ch[1] = { data.data() };
            juce::AudioBuffer<float> buffer(ch, 1, block);
            features.pushBlock(buffer);
        }
    }
}

int main()
{
    for (const double bpm : { 90.0, 110.0, 128.0, 140.0, 174.0 })
    {
        for (const double sr : { 44100.0, 48000.0 })
        {
            AudioFeatures features;
            features.prepare(sr);
            Groove groove;
            groove.bpm = bpm;
            groove.sr = sr;
            run(features, [&] { return groove.next(); }, 10.0, sr);
            const auto& tempo = features.getTempo();
            const float error = std::abs(tempo.getBpm() - (float) bpm);
            double phaseError = std::abs(tempo.getPhase() - groove.phaseAt());
            phaseError = std::min(phaseError, 1.0 - phaseError);
            std::cout << "  " << bpm << " BPM @" << sr << ": detected " << tempo.getBpm()
                      << " conf " << tempo.getConfidence() << " phase err " << phaseError << '\n';
            check(tempo.isLocked() && error < 1.5f, "Locks to " + std::to_string((int) bpm) + " BPM groove");
            check(phaseError < 0.1, "Beat phase aligned at " + std::to_string((int) bpm) + " BPM");
        }
    }

    {
        AudioFeatures features;
        features.prepare(48000.0);
        Groove groove;
        groove.snare = groove.hats = false;
        groove.bpm = 128.0;
        run(features, [&] { return groove.next(); }, 8.0, 48000.0);
        check(features.getTempo().isLocked() && std::abs(features.getTempo().getBpm() - 128.0f) < 1.5f,
              "Bare four-on-the-floor kick locks");
        run(features, [] { return 0.0f; }, 4.0, 48000.0);
        check(!features.getTempo().isLocked(), "Tracker lets go after silence");
    }

    {
        AudioFeatures features;
        features.prepare(48000.0);
        juce::Random rng(3);
        run(features, [&] { return 0.3f * (rng.nextFloat() * 2.0f - 1.0f); }, 10.0, 48000.0);
        check(!features.getTempo().isLocked(), "Steady noise does not fake a tempo");
    }

    {
        AudioFeatures features;
        features.prepare(48000.0);
        Groove groove;
        float lowSeen = 0, highSeen = 0, kickSeen = 0;
        std::vector<float> data(512);
        for (int b = 0; b < 400; ++b)
        {
            for (auto& s : data) s = groove.next();
            float* ch[1] = { data.data() };
            juce::AudioBuffer<float> buffer(ch, 1, 512);
            features.pushBlock(buffer);
            lowSeen = std::max(lowSeen, features.getLow());
            highSeen = std::max(highSeen, features.getHigh());
            kickSeen = std::max(kickSeen, features.getKick());
        }
        std::cout << "  bands: low " << lowSeen << " high " << highSeen << " kick " << kickSeen << '\n';
        check(lowSeen > 0.6f && highSeen > 0.25f, "Band levels respond to the groove");
        check(kickSeen > 0.9f, "Kick detector fires on the bass drum");
        juce::Random rng(9);
        run(features, [&] { return 0.2f * (rng.nextFloat() * 2.0f - 1.0f); }, 1.0, 48000.0);
        check(features.getHigh() > features.getLow(), "White noise reads brighter than bass");
    }

    return passed ? 0 : 1;
}
