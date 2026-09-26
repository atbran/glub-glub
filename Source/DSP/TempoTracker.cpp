#include "TempoTracker.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kMinBpm = 60.0f, kMaxBpm = 200.0f;

    // Broad preference for dance tempos: one octave either side of 120 costs ~40%.
    float tempoPrior(float bpm)
    {
        const float octaves = std::log2(bpm / 120.0f);
        return std::exp(-0.5f * octaves * octaves);
    }

    double wrap01(double x) { return x - std::floor(x); }
}

void TempoTracker::prepare(double framesPerSecond)
{
    fps = framesPerSecond > 1.0 ? framesPerSecond : 100.0;
    reset();
}

void TempoTracker::reset()
{
    history.fill(0.0f);
    acf.fill(0.0f);
    writePos = filled = sinceEstimate = 0;
    phase = 0.0;
    beatCount = 0;
    bpm = confidence = candidateBpm = 0.0f;
    candidateHits = weakEstimates = 0;
    locked = false;
}

float TempoTracker::at(int framesAgo) const noexcept
{
    int i = writePos - 1 - framesAgo;
    while (i < 0) i += kHistory;
    return history[(size_t) (i % kHistory)];
}

void TempoTracker::pushFrame(float onsetStrength)
{
    history[(size_t) writePos] = std::max(0.0f, onsetStrength);
    writePos = (writePos + 1) % kHistory;
    filled = std::min(kHistory, filled + 1);

    if (locked)
    {
        phase += bpm / (60.0 * fps);
        if (phase >= 1.0) { phase -= 1.0; ++beatCount; }
    }

    if (++sinceEstimate >= (int) (fps * 0.5))
    {
        sinceEstimate = 0;
        estimate();
    }
}

float TempoTracker::measurePhase(double period) const noexcept
{
    // Score each candidate "last beat was phi frames ago" against a comb of
    // earlier beats. Recent beats weigh more so tempo drift is forgiven.
    const int combs = std::min(8, (int) ((filled - 2) / period));
    int bestPhi = 0;
    float bestScore = -1.0f;
    for (int phi = 0; phi < (int) std::ceil(period); ++phi)
    {
        float score = 0.0f, weight = 1.0f;
        for (int k = 0; k < combs; ++k)
        {
            const int idx = (int) std::lround(phi + k * period);
            score += weight * (at(idx) + 0.5f * (at(idx + 1) + (idx > 0 ? at(idx - 1) : 0.0f)));
            weight *= 0.85f;
        }
        if (score > bestScore) { bestScore = score; bestPhi = phi; }
    }
    return (float) (bestPhi / period);
}

void TempoTracker::estimate()
{
    if (filled < (int) (fps * 3.0)) return;

    const int n = filled;
    double mean = 0.0;
    for (int i = 0; i < n; ++i) mean += at(i);
    mean /= n;
    double energy = 0.0;
    for (int i = 0; i < n; ++i) { const double d = at(i) - mean; energy += d * d; }
    energy /= n;

    const int minLag = std::max(2, (int) std::floor(fps * 60.0 / kMaxBpm));
    const int maxLag = std::min(kMaxLag, (int) std::ceil(fps * 60.0 / kMinBpm));

    int bestLag = -1;
    float bestScore = 0.0f;
    if (energy > 1e-10)
    {
        for (int lag = minLag - 1; lag <= maxLag + 1; ++lag)
        {
            double sum = 0.0;
            for (int i = 0; i + lag < n; ++i)
                sum += (at(i) - mean) * (at(i + lag) - mean);
            acf[(size_t) lag] = (float) (sum / (n - lag) / energy);
        }
        for (int lag = minLag; lag <= maxLag; ++lag)
        {
            const float a = acf[(size_t) lag];
            // Only local maxima count; a slope next to a peak is not a tempo.
            if (a <= acf[(size_t) lag - 1] || a < acf[(size_t) lag + 1]) continue;
            const float score = a * tempoPrior((float) (fps * 60.0 / lag));
            if (score > bestScore) { bestScore = score; bestLag = lag; }
        }
    }

    // Old onsets linger in the history; the groove only counts if it is still playing.
    float recentPeak = 0.0f, overallPeak = 0.0f;
    const int recentFrames = (int) (fps * 1.5);
    for (int i = 0; i < n; ++i)
    {
        overallPeak = std::max(overallPeak, at(i));
        if (i < recentFrames) recentPeak = std::max(recentPeak, at(i));
    }
    const bool stillPlaying = recentPeak > 0.15f * overallPeak && recentPeak > 1.0e-4f;

    confidence = bestLag > 0 && stillPlaying ? std::clamp(acf[(size_t) bestLag], 0.0f, 1.0f) : 0.0f;
    if (confidence < 0.10f)
    {
        if (++weakEstimates >= (stillPlaying ? 4 : 2)) { locked = false; candidateHits = 0; }
        return;
    }
    weakEstimates = 0;

    // Parabolic refinement: the true period rarely lands on a whole frame.
    const float y0 = acf[(size_t) bestLag - 1], y1 = acf[(size_t) bestLag], y2 = acf[(size_t) bestLag + 1];
    const float denom = y0 - 2.0f * y1 + y2;
    const float delta = denom < 0.0f ? std::clamp(0.5f * (y0 - y2) / denom, -0.5f, 0.5f) : 0.0f;
    double lag = bestLag + delta;
    float estimateBpm = (float) (fps * 60.0 / lag);

    // Half-time guard: a strong peak at half the lag means we found the bar pulse.
    if (estimateBpm < 95.0f)
    {
        const int half = (int) std::lround(lag * 0.5);
        if (half >= minLag && acf[(size_t) half] > 0.5f * y1)
        {
            estimateBpm *= 2.0f;
            lag *= 0.5;
        }
    }

    auto agrees = [](float a, float b) { return std::abs(a - b) < 0.04f * b; };
    if (!locked)
    {
        candidateHits = agrees(estimateBpm, candidateBpm) ? candidateHits + 1 : 1;
        candidateBpm = estimateBpm;
        if (candidateHits >= 2)
        {
            locked = true;
            bpm = estimateBpm;
            phase = measurePhase(60.0 * fps / bpm);
            candidateHits = 0;
        }
        return;
    }

    if (agrees(estimateBpm, bpm))
    {
        bpm += 0.35f * (estimateBpm - bpm);
        candidateHits = 0;
    }
    else
    {
        // A genuine tempo change must repeat before we abandon the groove.
        candidateHits = agrees(estimateBpm, candidateBpm) ? candidateHits + 1 : 1;
        candidateBpm = estimateBpm;
        if (candidateHits >= 3) { bpm = estimateBpm; candidateHits = 0; }
    }

    const double measured = measurePhase(60.0 * fps / bpm);
    double error = wrap01(measured - phase + 0.5) - 0.5;
    phase = wrap01(phase + 0.45 * error);
}
