#pragma once
#include <array>

// Allocation-free tempo + beat-phase tracker for when no host clock exists.
// Feed one onset-strength value per analysis frame (~100 fps). Every half
// second it autocorrelates the last ~7.7 s of onsets to find the beat period,
// then comb-filters the history to find where the beats land. A soft
// phase-locked loop keeps the reported phase continuous between estimates.
class TempoTracker
{
public:
    static constexpr int kHistory = 768;
    static constexpr int kMaxLag = 128;

    void prepare(double framesPerSecond);
    void reset();
    void pushFrame(float onsetStrength);

    bool isLocked() const noexcept { return locked; }
    float getBpm() const noexcept { return bpm; }
    float getPhase() const noexcept { return (float) phase; } // 0..1, 0 = on the beat
    float getConfidence() const noexcept { return confidence; }
    long long getBeatCount() const noexcept { return beatCount; }

private:
    void estimate();
    float at(int framesAgo) const noexcept;
    float measurePhase(double periodFrames) const noexcept;

    std::array<float, kHistory> history {};
    std::array<float, kMaxLag + 2> acf {};
    int writePos = 0, filled = 0, sinceEstimate = 0;
    double fps = 100.0;
    double phase = 0.0;
    long long beatCount = 0;
    float bpm = 0.0f, confidence = 0.0f;
    float candidateBpm = 0.0f;
    int candidateHits = 0, weakEstimates = 0;
    bool locked = false;
};
