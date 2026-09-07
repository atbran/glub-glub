#include "Bubbles.h"
#include <cmath>
#include <algorithm>

Bubbles::Bubbles() { pool.reserve(40); }

void Bubbles::update(float energy, juce::Point<float> mouth)
{
    // idle-heavy: spawn more when calm
    float spawnP = energy < 0.3f ? 0.25f : 0.05f;
    if (enabled && pool.size() < 36 && rng.nextFloat() < spawnP)
        pool.push_back({ mouth.x + rng.nextFloat() * 8 - 4, mouth.y, 2 + rng.nextFloat() * 4, 20 + rng.nextFloat() * 30, rng.nextFloat() * 6.28f, 0.8f });

    for (auto& p : pool)
    {
        p.y -= p.speed * 0.016f;
        p.wobble += 0.06f;
        p.x += std::sin(p.wobble) * 0.4f;
        p.alpha -= 0.002f;
    }
    pool.erase(std::remove_if(pool.begin(), pool.end(),
        [](const P& p) { return p.alpha <= 0 || p.y < -10; }), pool.end());
    repaint();
}

void Bubbles::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0x99BEE9E8));
    for (auto& p : pool)
    {
        g.setOpacity(juce::jlimit(0.0f, 1.0f, p.alpha));
        g.drawEllipse(p.x, p.y, p.r * 2, p.r * 2, 1.5f);
        g.setOpacity(juce::jlimit(0.0f, 1.0f, p.alpha) * 0.5f);
        g.fillEllipse(p.x + p.r * 0.6f, p.y + p.r * 0.6f, p.r * 0.5f, p.r * 0.5f);
    }
    g.setOpacity(1.0f);
}
