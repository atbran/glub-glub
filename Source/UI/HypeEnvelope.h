#pragma once
#include <algorithm>
#include <cmath>

// UI-only ballistics. Loudness comes from the audio envelope before the vibe
// sensitivity control; a transient or sensitivity boost alone cannot peg hype.
class HypeEnvelope
{
public:
    float update(float loudness, float pulse, float feed, float dt)
    {
        dt = std::clamp(dt, 0.0f, 0.05f);
        loudness = std::clamp(loudness, 0.0f, 1.0f);
        const float level = std::clamp((loudness - 0.18f) / 0.68f, 0.0f, 1.0f);
        float target = std::pow(level, 1.6f);
        target = std::min(1.0f, target + 0.06f * pulse * level);
        target = std::max(target, std::clamp(feed, 0.0f, 1.0f) * 0.72f);
        const bool drop = loudness > 0.72f && loudness > baseline + 0.20f;
        const float seconds = target > value ? (drop ? 0.40f : 1.25f) : 1.5f;
        value += (target - value) * (1.0f - std::exp(-dt / seconds));
        baseline += (loudness - baseline) * (1.0f - std::exp(-dt / 3.0f));
        return value;
    }
private:
    float value = 0.0f, baseline = 0.0f;
};
