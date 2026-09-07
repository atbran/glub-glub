#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

class Bubbles : public juce::Component
{
public:
    Bubbles();
    void setEnabled(bool on) { enabled = on; }
    void update(float energy, juce::Point<float> mouth);
    void burst(juce::Point<float> mouth);
    void paint(juce::Graphics& g) override;

private:
    struct P { float x, y, r, speed, wobble, alpha; };
    std::vector<P> pool;
    bool enabled = true;
    juce::Random rng;
};
