#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class SpeechBox : public juce::Component
{
public:
    SpeechBox();
    void update(double nowSec, float energy, int intensity, float speechRateSec, juce::Random& rng);
    void paint(juce::Graphics& g) override;
    bool hasText() const { return alpha > 0.01f; }

private:
    juce::String current;
    float alpha = 0.0f;
    double lastChange = -100.0;
    double nextAt = 8.0;
    double shownAt = 0.0;

    static juce::String pick(int intensity, float energy, juce::Random& rng);
};
