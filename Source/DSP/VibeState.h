#pragma once
#include <atomic>

struct VibeState
{
    std::atomic<float> energy { 0.0f };      // 0..1 smoothed RMS
    std::atomic<float> brightness { 0.0f };  // 0..1 ZCR/HF proxy
    std::atomic<float> beatPulse { 0.0f };   // 0..1 onset pulse (decays)
    std::atomic<float> beatPhase { 0.0f };   // 0..1 host beat phase, -1 if unknown
    std::atomic<float> bpm { 0.0f };         // 0 if unknown
    std::atomic<int> intensity { 0 };        // 0 Low, 1 Med, 2 High
    std::atomic<int> barCount { -1 };

    // Manual feed boost (food shaker): 0..1. UI side writes it while feeding
    // and lets it decay after the feeding window; consumers treat it like a
    // hype override so the koi dances, the meter pegs and the disco drops.
    std::atomic<float> feedBoost { 0.0f };

    VibeState() { beatPhase.store(-1.0f); }
};
