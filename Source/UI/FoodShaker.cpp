#include "FoodShaker.h"
#include <cmath>

namespace
{
    // 3-tone pixel-art palette, matching the koi / disco ball style.
    constexpr unsigned OUTLINE = 0xFF261712;
    constexpr unsigned MAIN    = 0xFFD64530;   // canister red-orange
    constexpr unsigned DARK    = 0xFF8F2418;   // shade
    constexpr unsigned LIGHT   = 0xFFF08A5D;   // highlight
    constexpr unsigned LABEL   = 0xFFF5EBD0;   // cream label
    constexpr unsigned LID     = 0xFF4A4F57;   // metal lid

    // 12 x 16 art. '.' transparent, O outline, M main, L light,
    // A label cream, F label mark (tiny fish), I lid metal, H lid highlight.
    const char* art[FoodShaker::GRID_H] = {
        "............",
        ".IIIIIIIIII.",
        ".IHHHHHHHHI.",
        ".OOOOOOOOOO.",
        ".OMLMMMMMMO.",
        ".OMLMMMMMMO.",
        ".OMLMMMMMMO.",
        ".OAAAAAAAAO.",
        ".OAFAAAAAAO.",
        ".OAAAAAAAAO.",
        ".OAAAAAAAAO.",
        ".OAAAAAAAAO.",
        ".OMLMMMMMMO.",
        ".OMLMMMMMMO.",
        ".OOOOOOOOOO.",
        "............",
    };

    juce::Colour colourFor(char c)
    {
        switch (c)
        {
            case 'O': return juce::Colour(OUTLINE);
            case 'M': return juce::Colour(MAIN);
            case 'D': return juce::Colour(DARK);
            case 'L': return juce::Colour(LIGHT);
            case 'A': return juce::Colour(LABEL);
            case 'F': return juce::Colour(MAIN);
            case 'H': return juce::Colour(LIGHT);
            case 'I': return juce::Colour(LID);
            default:  return juce::Colour();
        }
    }
}

FoodShaker::FoodShaker()
{
    setMouseCursor(juce::MouseCursor(juce::MouseCursor::DraggingHandCursor));
}

juce::Rectangle<int> FoodShaker::canRectLocal() const
{
    auto origin = getBounds().getTopLeft();
    return juce::Rectangle<int>(canPos.x - origin.x, canPos.y - origin.y,
                                GRID_W * CELL, GRID_H * CELL);
}

bool FoodShaker::hitTest(int x, int y)
{
    // only the canister grabs the mouse; the rest of the tank passes through
    return canRectLocal().contains(x, y);
}

void FoodShaker::paint(juce::Graphics& g)
{
    const auto origin = getBounds().getTopLeft();

    // pellets live in parent coordinates; convert to local
    for (auto& p : pellets)
    {
        double age = (lastFrameTime - p.bornAt);
        float a = (float) juce::jlimit(0.0, 1.0, 1.0 - age / 5.0);
        juce::Colour c = p.shape == 0 ? juce::Colour(0xFFFFC93C)
                       : p.shape == 1 ? juce::Colour(0xFFFF8A3D)
                                      : juce::Colour(0xFFC97B3C);
        g.setColour(c.withAlpha(a));
        int px = (int) std::lround(p.pos.x) - origin.x;
        int py = (int) std::lround(p.pos.y) - origin.y;
        // 3x3 / 2x3 / 4x2 flake shapes, all on the integer grid
        if (p.shape == 0)      g.fillRect(px - 1, py - 1, 3, 3);
        else if (p.shape == 1) g.fillRect(px - 1, py - 1, 2, 3);
        else                   g.fillRect(px - 2, py, 4, 2);
    }

    // canister, integer-snapped cells
    int ox = canPos.x - origin.x;
    int oy = canPos.y - origin.y;
    for (int y = 0; y < GRID_H; ++y)
    {
        const char* row = art[y];
        for (int x = 0; x < GRID_W; ++x)
        {
            char c = row[x];
            if (c == '.') continue;
            g.setColour(colourFor(c));
            g.fillRect(ox + x * CELL, oy + y * CELL, CELL, CELL);
        }
    }
}

void FoodShaker::mouseDown(const juce::MouseEvent& e)
{
    dragging = true;
    auto parentPos = e.getEventRelativeTo(getParentComponent()).getPosition();
    dragOffset = parentPos - canPos;
    lastPos = parentPos.toFloat();
    lastMoveTime = lastFrameTime;
    toFront(false);
}

void FoodShaker::mouseDrag(const juce::MouseEvent& e)
{
    if (! dragging) return;

    auto parentPos = e.getEventRelativeTo(getParentComponent()).getPosition();
    canPos = parentPos - dragOffset;

    double now = lastFrameTime;
    float dt = (float) juce::jlimit(0.001, 0.1, now - lastMoveTime);
    auto p = parentPos.toFloat();
    float dx = p.x - lastPos.x;
    speed = speed * 0.7f + 0.3f * (std::abs(dx) / dt);

    // shake = fast back-and-forth horizontal motion
    int dir = dx > 1.5f ? 1 : (dx < -1.5f ? -1 : lastDir);
    if (dir != 0 && lastDir != 0 && dir != lastDir && speed > 350.0f)
        ++shakeBeats;
    lastDir = dir;
    shaking = speed > 500.0f && shakeBeats > 1;

    lastPos = p;
    lastMoveTime = now;

    // sprinkle while held over the water; shaking dumps food faster
    auto centre = canRectLocal().getCentre() + getBounds().getTopLeft();
    double interval = shaking ? 0.07 : 0.22;
    if (centre.x >= waterRect.getX() && centre.x <= waterRect.getRight()
        && centre.y <= waterRect.getBottom()
        && now - lastSpawnTime > interval)
        spawnPellet(now);

    repaint();
}

void FoodShaker::mouseUp(const juce::MouseEvent&)
{
    dragging = false;
    shaking = false;
    speed = 0.0f;
    shakeBeats = 0;
}

void FoodShaker::spawnPellet(double nowSec)
{
    lastSpawnTime = nowSec;
    // food drops from the canister mouth (bottom-centre) in parent coords
    auto mouth = canRectLocal().getCentre() + getBounds().getTopLeft();
    Pellet p;
    p.pos = { (float) (mouth.x + juce::Random::getSystemRandom().nextInt(9) - 4),
              (float) (canPos.y + GRID_H * CELL) };
    p.vel = { juce::Random::getSystemRandom().nextFloat() * 24.0f - 12.0f, 10.0f };
    p.bornAt = nowSec;
    p.shape = juce::Random::getSystemRandom().nextInt(3);
    pellets.push_back(p);
}

void FoodShaker::update(double nowSec)
{
    float dt = (float) juce::jlimit(0.0, 0.1, nowSec - lastFrameTime);
    bool firstFrame = lastFrameTime <= 0.0;
    lastFrameTime = nowSec;
    if (firstFrame) return;

    // pellet physics
    for (auto& p : pellets)
    {
        p.vel.y = juce::jmin(p.vel.y + 240.0f * dt, 140.0f);
        p.vel.x *= 0.995f;
        p.pos += p.vel * dt;
        p.pos.x += std::sin((float) (nowSec * 3.0 + p.bornAt)) * 6.0f * dt;   // drift
    }
    pellets.erase(std::remove_if(pellets.begin(), pellets.end(),
                                 [&](const Pellet& p)
                                 {
                                     return nowSec - p.bornAt > 5.0
                                         || p.pos.y > waterRect.getBottom() + 10.0f;
                                 }),
                  pellets.end());

    // feeding flag: held over water with recent spawns
    feeding = dragging && ! pellets.empty();

    // ease the canister back home after release
    if (! dragging && canPos != home)
    {
        juce::Point<float> cf ((float) canPos.x, (float) canPos.y);
        juce::Point<float> hf ((float) home.x, (float) home.y);
        if (cf.getDistanceFrom(hf) < 2.0f)
        {
            canPos = home;
        }
        else
        {
            // smooth exponential ease
            float t = juce::jlimit(0.0f, 1.0f, 14.0f * dt);
            auto nf = cf + (hf - cf) * t;
            canPos = { (int) std::lround(nf.x), (int) std::lround(nf.y) };
        }
        repaint();
    }

    if (! pellets.empty())
        repaint();
}

void FoodShaker::setHome(juce::Point<int> homeTopLeft)
{
    home = homeTopLeft;
    if (! homeInit)
    {
        canPos = home;
        homeInit = true;
    }
}

void FoodShaker::setWaterRect(juce::Rectangle<int> r) { waterRect = r; }
