#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

class KoiFish : public juce::Component
{
public:
    enum class MoveType { None, Worm, Roll, Spin, Flip, Shuffle, HeadBop, Shimmy, FigureEight, Twerk };

    KoiFish();
    void setVibe(float energy, float brightness, float pulse, float beatPhase, int intensity, int barCount, float feedBoost, float hypeLevel = 0.0f, float deltaSeconds = 1.0f / 60.0f, float bpm = 120.0f);
    void setGlassesOn(bool on) { glassesOn = on; }
    void setGentleMotion(bool on) { gentleMotion = on; }
    void setMouseTarget(juce::Point<float> pos, bool inWindow);
    void triggerStartle(juce::Point<float> tapPos);
    void triggerMove(MoveType move);
    juce::Point<float> getMouthPosition() const;
    void paint(juce::Graphics& g) override;

    static constexpr int GRID_W = 36;
    static constexpr int GRID_H = 32;

private:
    friend struct KoiMotionChecks;
    float energy = 0.0f, brightness = 0.0f, pulse = 0.0f, beatPhase = -1.0f;
    int intensity = 0;
    float hype = 0.0f;
    double time = 0.0;
    double swimPhase = 0.0, driftPhase = 0.0, freeBeat = 0.0;
    float danceBeat = 0.0f, moveCooldown = 0.0f;
    float previousBeat = -1.0f, beatIdleTime = 0.0f, beatStep = 0.0f;
    int lastDanceBar = -1, danceIndex = 0;
    MoveType extraMove = MoveType::None;
    float extraT = 0.0f;
    bool glassesOn = false, gentleMotion = false;
    bool isMoveBusy() const;
    static juce::Point<float> figureEightPath(float progress, float width, float height);
    bool facingRight = true;
    float tailBurst = 0.0f;
    float flipT = 0.0f;
    bool flipActive = false, flipMid = false;
    float calmTime = 0.0f;
    bool sleepy = false;
    float partyT = 0.0f;
    bool partyActive = false;

    MoveType queuedMove = MoveType::None;

    // Dance Move 1: The Worm (Breakdance wave ripple)
    bool wormActive = false;
    float wormT = 0.0f;

    // Dance Move 2: The Barrel Roll (3D corkscrew loop)
    bool rollActive = false;
    float rollT = 0.0f;

    // Fun Addition 1: Deal With It Party Shades
    float shadesT = 0.0f;

    // Fun Addition 2: Cursor tracking & startle
    juce::Point<float> mousePos { -1.0f, -1.0f };
    bool mouseActive = false;
    float gazeAngle = 0.0f;
    float startleT = 0.0f;
    juce::Point<float> startleVec { 0.0f, 0.0f };
    float fishCx = 250.0f, fishCy = 260.0f;
    float mouseFlipCooldown = 0.0f;
    int eyeGX = 26, eyeGY = 8;

    int mouthGX = 33, mouthGY = 13;
    juce::Point<float> mouthPos { 0, 0 };
    juce::Image spriteImg;

    void buildGrid(std::vector<char>& grid);
};
