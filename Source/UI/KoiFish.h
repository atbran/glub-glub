#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

class KoiFish : public juce::Component
{
public:
    KoiFish();
    void setVibe(float energy, float brightness, float pulse, float beatPhase, int intensity, int barCount);
    juce::Point<float> getMouthPosition() const;
    void paint(juce::Graphics& g) override;

    static constexpr int GRID_W = 36;
    static constexpr int GRID_H = 24;

private:
    float energy = 0.0f, brightness = 0.0f, pulse = 0.0f, beatPhase = -1.0f;
    int intensity = 0;
    double time = 0.0;
    bool facingRight = true;
    int lastBarCount = -1;
    float spinAngle = 0.0f, spinVelocity = 0.0f;
    float tailBurst = 0.0f;
    float flipT = 0.0f, flipCooldown = 0.0f;
    bool flipActive = false, flipMid = false;
    float calmTime = 0.0f;
    bool sleepy = false;
    float partyT = 0.0f;
    bool partyActive = false;
    int lastPartyBar = -1;
    int mouthGX = 33, mouthGY = 13;
    juce::Point<float> mouthPos { 0, 0 };
    juce::Image spriteImg;

    void buildGrid(std::vector<char>& grid);
};
