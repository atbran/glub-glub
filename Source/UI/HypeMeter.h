#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class HypeMeter : public juce::Component
{
public:
    HypeMeter();
    void setHype(float h);
    void paint(juce::Graphics& g) override;

private:
    float hype = 0.0f;
};
