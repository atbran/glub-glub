#pragma once
#include <atomic>

struct VibeState
{
    std::atomic<float> energy { 0.0f };      // 0..1 smoothed RMS
    std::atomic<float> loudness { 0.0f };    // envelope before user sensitivity
    std::atomic<float> brightness { 0.0f };  // 0..1 ZCR/HF proxy
    std::atomic<float> beatPulse { 0.0f };   // 0..1 onset pulse (decays)
    std::atomic<float> beatPhase { 0.0f };   // 0..1 host beat phase, -1 if unknown
    std::atomic<float> bpm { 0.0f };         // 0 if unknown
    std::atomic<int> intensity { 0 };        // 0 Low, 1 Med, 2 High
    std::atomic<int> barCount { -1 };

    // Three-band spectrum (0..1) and a bass-only kick pulse for the tank scene.
    std::atomic<float> low { 0.0f }, mid { 0.0f }, high { 0.0f };
    std::atomic<float> kick { 0.0f };

    // Where the beat grid comes from: the DAW transport, or Glub's own ears.
    enum TempoSource { tempoNone = 0, tempoHost = 1, tempoDetected = 2 };
    std::atomic<int> tempoSource { tempoNone };
    std::atomic<float> tempoConfidence { 0.0f };

    // Manual feed boost (food shaker): 0..1. UI side writes it while feeding
    // and lets it decay after the feeding window; consumers treat it like a
    // dance boost; musical loudness is still required to max out the meter.
    std::atomic<float> feedBoost { 0.0f };

    VibeState() { beatPhase.store(-1.0f); }
};
