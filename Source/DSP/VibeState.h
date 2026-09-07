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
    std::atomic<int> barCount { 0 };

    VibeState() { beatPhase.store(-1.0f); }
};
