#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class HypeMeter : public juce::Component
{
public:
    HypeMeter();
    void setHype(float h);
    // source: 0 none, 1 host, 2 detected (matches VibeState::TempoSource).
    void setTempo(float bpm, int source, float beatPhase);
    void paint(juce::Graphics& g) override;

private:
    float hype = 0.0f;
    float bpm = 0.0f, beatPhase = -1.0f;
    int source = 0;
};
