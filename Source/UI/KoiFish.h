#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class KoiFish : public juce::Component
{
public:
    KoiFish();
    void setVibe(float energy, float brightness, float pulse, float beatPhase, int intensity);
    juce::Point<float> getMouthPosition() const;
    void paint(juce::Graphics& g) override;

private:
    float energy = 0.0f, brightness = 0.0f, pulse = 0.0f, beatPhase = -1.0f;
    int intensity = 0;
    double time = 0.0;
    bool facingRight = true;
    juce::Point<float> mouthPos { 0, 0 };

    void drawPixel(juce::Graphics& g, int gx, int gy, int pixel, juce::Colour c, float ox, float oy) const;
};
