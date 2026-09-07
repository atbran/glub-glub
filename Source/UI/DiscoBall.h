#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class DiscoBall : public juce::Component
{
public:
    DiscoBall();
    void update(float hypeLevel, double nowSec);
    void paint(juce::Graphics& g) override;

private:
    float level = 0.0f;
    float appear = 0.0f;
    bool showing = false;
    double shownAt = 0.0;
    double time = 0.0;
    double lastNow = 0.0;
    float aboveTime = 0.0f;
    float belowTime = 0.0f;
};
