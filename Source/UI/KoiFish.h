#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <optional>
#include <vector>

class KoiFish : public juce::Component
{
public:
    enum class MoveType { None, Worm, Roll, Spin, Flip, Shuffle, HeadBop, Shimmy, FigureEight, Twerk, Breakdance };

    KoiFish();
    void setVibe(float energy, float brightness, float pulse, float beatPhase, int intensity, int barCount, float feedBoost, float hypeLevel = 0.0f, float deltaSeconds = 1.0f / 60.0f, float bpm = 120.0f);
    void setGlassesOn(bool on) { glassesOn = on; }
    void setGentleMotion(bool on) { gentleMotion = on; }
    void setMouseTarget(juce::Point<float> pos, bool inWindow);
    void triggerStartle(juce::Point<float> tapPos);
    void triggerMove(MoveType move);
    bool isBreakdancing() const { return activeMove == MoveType::Breakdance; }
    MoveType getActiveMove() const { return activeMove; }

    // Food: the koi swims its mouth toward the target pellet; chomp() when it lands.
    void setFoodTarget(std::optional<juce::Point<float>> pellet) { foodTarget = pellet; }
    void chomp();
    bool isChasingFood() const { return foodTarget.has_value(); }
    float getFullness() const { return fullness; }
    bool consumeBreakdanceTriggered()
    {
        bool triggered = breakdanceJustTriggered;
        breakdanceJustTriggered = false;
        return triggered;
    }
    juce::Point<float> getFloorContactPos() const { return floorContactPos; }
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
    bool glassesOn = false, gentleMotion = false;
    bool isMoveBusy() const;
    bool doing(MoveType move) const { return activeMove == move; }
    static float moveBeats(MoveType move);
    static juce::Point<float> figureEightPath(float progress, float width, float height);
    bool facingRight = true;
    float tailBurst = 0.0f;
    bool flipMid = false;
    float calmTime = 0.0f;
    bool sleepy = false;

    // One move at a time; progress runs 0..1 over moveBeats() host beats.
    MoveType activeMove = MoveType::None;
    float moveT = 0.0f;
    MoveType queuedMove = MoveType::None;

    float breakdanceCooldown = 0.0f;
    bool breakdanceJustTriggered = false;
    juce::Point<float> floorContactPos { 0, 0 };

    // Feeding: roaming offset from the tank centre, spring-driven toward food.
    std::optional<juce::Point<float>> foodTarget;
    juce::Point<float> roam { 0, 0 }, roamVel { 0, 0 };
    float chaseAmt = 0.0f, chompT = 0.0f, fullness = 0.0f;
    struct Heart { float x = 0, y = 0, age = 1.0f; };
    std::array<Heart, 6> hearts {};
    int nextHeart = 0;

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
    juce::Point<float> spriteOrigin { 0, 0 };

    void buildGrid(std::vector<char>& grid);
};
