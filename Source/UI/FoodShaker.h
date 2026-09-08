#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

// Fish-food shaker canister. The component covers the whole tank/water area
// (so falling pellets are not clipped), but hitTest only accepts clicks on
// the canister itself. The canister position lives in parent coordinates.
class FoodShaker : public juce::Component
{
public:
    FoodShaker();

    void paint(juce::Graphics& g) override;
    bool hitTest(int x, int y) override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    // Called from the editor's 60 Hz timer.
    void update(double nowSec);
    void setHome(juce::Point<int> homeTopLeft);
    void setWaterRect(juce::Rectangle<int> r);
    bool isFeeding() const { return feeding; }

    static constexpr int CELL = 4;   // pixel-art cell size
    static constexpr int GRID_W = 12;
    static constexpr int GRID_H = 16;

private:
    struct Pellet
    {
        juce::Point<float> pos;   // parent coordinates
        juce::Point<float> vel;
        double bornAt = 0.0;
        int shape = 0;            // pellet/flake variation
    };

    juce::Point<int> home { 0, 0 };     // canister top-left, parent coords
    juce::Point<int> canPos { 0, 0 };   // current canister top-left, parent coords
    bool homeInit = false;
    juce::Rectangle<int> waterRect;
    bool dragging = false;
    bool feeding = false;
    bool shaking = false;
    juce::Point<int> dragOffset { 0, 0 };

    // shake detection
    juce::Point<float> lastPos { 0, 0 };
    double lastMoveTime = 0.0;
    float speed = 0.0f;          // px/sec, smoothed
    int lastDir = 0;             // -1 / 0 / +1 horizontal direction
    int shakeBeats = 0;

    double lastSpawnTime = 0.0;
    double lastFrameTime = 0.0;
    std::vector<Pellet> pellets;

    void spawnPellet(double nowSec);
    juce::Rectangle<int> canRectLocal() const;
};
