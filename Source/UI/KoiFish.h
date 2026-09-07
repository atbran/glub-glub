#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class KoiFish : public juce::Component
{
public:
    KoiFish();
    void setVibe(float energy, float brightness, float pulse, float beatPhase, int intensity, int barCount);
    juce::Point<float> getMouthPosition() const;
    void paint(juce::Graphics& g) override;

    static constexpr int SPRITE_W = 34;
    static constexpr int SPRITE_H = 22;

private:
    float energy = 0.0f, brightness = 0.0f, pulse = 0.0f, beatPhase = -1.0f;
    int intensity = 0;
    double time = 0.0;
    bool facingRight = true; // sprite is tail-left, head-right; mirrored at draw time
    int lastBarCount = -1;
    float spinAngle = 0.0f;
    float spinVelocity = 0.0f;
    juce::Point<float> mouthPos { 0, 0 };

    char styleAt(int gx, int gy) const;       // '.', 'B', 'W', 'R', 'K', 'T'
    bool maskAt(int gx, int gy) const;
    float undulationOffset(int gx, float swimSpeed, float amp) const;
    void drawPixel(juce::Graphics& g, float sx, float sy, int pixel, juce::Colour c, float ox, float oy) const;
};
