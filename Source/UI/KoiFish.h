#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class KoiFish : public juce::Component
{
public:
    KoiFish();
    void setVibe(float energy, float brightness, float pulse, float beatPhase, int intensity, int barCount);
    juce::Point<float> getMouthPosition() const;
    void paint(juce::Graphics& g) override;

private:
    float energy = 0.0f, brightness = 0.0f, pulse = 0.0f, beatPhase = -1.0f;
    int intensity = 0;
    double time = 0.0;
    bool facingRight = true;
    int lastBarCount = -1;
    float spinAngle = 0.0f;
    float spinVelocity = 0.0f;
    juce::Point<float> mouthPos { 0, 0 };

    void drawPixel(juce::Graphics& g, int gx, int gy, int pixel, juce::Colour c, float ox, float oy) const;
};
