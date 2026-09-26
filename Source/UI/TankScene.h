#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <vector>

// Everything behind the koi: water, sand, rocks, a sunken chest, seaweed that
// pumps with the bass, coral that glows with the mids, light rays and plankton
// that sparkle with the highs, a school of tank mates, plus the party layer
// (disco lasers, tap ripples and the beat tint).
class TankScene : public juce::Component
{
public:
    enum class Theme { Lagoon = 0, Midnight, Sunset };

    struct Frame
    {
        float low = 0, mid = 0, high = 0, kick = 0, pulse = 0;
        float partyGlow = 0, partyHue = 0.55f;
        float discoAppear = 0;
        juce::Point<float> discoCentre;
        juce::Point<float> koi;
        bool tankMates = true;
    };

    TankScene();
    void setTheme(Theme t, float hueShift);
    // Height of the swimmable water; the sand bed starts just above it.
    void setFloorY(float y) { floorY = y; cache = {}; }
    void update(const Frame& frame, double nowSec, float dt);
    void addRipple(juce::Point<float> p);
    juce::Point<float> getChestMouth() const { return chestMouth; }
    bool consumeChestBurp();

    void paint(juce::Graphics& g) override;
    void resized() override { cache = {}; }

private:
    struct Palette
    {
        juce::Colour waterTop, waterBottom, sand, sandDark, sandLight, rock, rockDark,
                     weed, weedLight, coral, coralLight, ray, glow, speck;
        float glowBoost = 0.0f; // Midnight lets band glows bloom much more.
    };
    static Palette paletteFor(Theme t, float hueShift);
    void renderStatic();
    void drawSeaweed(juce::Graphics& g, float x, float baseY, int segments, float phase, bool front);
    void updateTetras(float dt);

    Theme theme = Theme::Lagoon;
    float hue = 0.0f;
    Palette pal = paletteFor(Theme::Lagoon, 0.0f);
    juce::Image cache;
    float floorY = 400.0f;
    int cell = 4;
    juce::Point<float> chestMouth, coralCentre;

    Frame frame;
    double time = 0.0;
    float weedKick = 0.0f;
    bool chestBurp = false;
    int kickCount = 0;
    bool kickHeld = false;

    struct Ripple { float x, y, radius, alpha; };
    std::vector<Ripple> ripples;

    struct Speck { float x, y, speed, phase; };
    std::array<Speck, 36> specks {};

    struct Tetra { juce::Point<float> pos, vel; float phase; };
    std::array<Tetra, 7> tetras {};
    juce::Point<float> schoolGoal;
    float goalTimer = 0.0f, dartCooldown = 0.0f;
    bool tetrasPlaced = false;
    juce::Random rng { 42 };
};
