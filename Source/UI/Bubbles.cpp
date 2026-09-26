#include "Bubbles.h"
#include <cmath>
#include <algorithm>

Bubbles::Bubbles() { pool.reserve(40); }

void Bubbles::update(float energy, juce::Point<float> mouth)
{
    // idle-heavy: spawn more when calm
    float spawnP = energy < 0.3f ? 0.22f : 0.05f;
    if (enabled && pool.size() < 36 && rng.nextFloat() < spawnP)
        pool.push_back({ mouth.x + rng.nextFloat() * 8 - 4, mouth.y, 2 + rng.nextFloat() * 4, 15 + rng.nextFloat() * 22, rng.nextFloat() * 6.28f, 0.8f });

    for (auto& p : pool)
    {
        p.y -= p.speed * 0.016f;
        p.wobble += 0.045f;
        p.x += std::sin(p.wobble) * 0.4f;
        p.alpha -= 0.0016f;
    }
    pool.erase(std::remove_if(pool.begin(), pool.end(),
        [](const P& p) { return p.alpha <= 0 || p.y < -10; }), pool.end());

    const float dt = 1.0f / 60.0f;
    for (auto& c : confettiPool)
    {
        c.vy += 260.0f * dt;               // gravity, softened by water drag
        c.vx *= 0.97f; c.vy *= 0.97f;
        c.x += c.vx * dt; c.y += c.vy * dt;
        c.spin += 9.0f * dt;
        c.life -= dt / 3.0f;
    }
    confettiPool.erase(std::remove_if(confettiPool.begin(), confettiPool.end(),
        [this](const Confetto& c) { return c.life <= 0.0f || c.y > (float) getHeight() + 10.0f; }), confettiPool.end());
    repaint();
}

void Bubbles::burst(juce::Point<float> mouth)
{
    if (!enabled || pool.size() > 32) return;
    for (int i = 0; i < 3; ++i)
        pool.push_back({ mouth.x + rng.nextFloat() * 10 - 5, mouth.y + rng.nextFloat() * 4, 2 + rng.nextFloat() * 3, 24 + rng.nextFloat() * 26, rng.nextFloat() * 6.28f, 0.9f });
}

void Bubbles::vortex(juce::Point<float> center)
{
    if (!enabled || pool.size() > 30) return;
    for (int i = 0; i < 8; ++i)
    {
        float angle = rng.nextFloat() * 6.2831853f;
        float dist = 6.0f + rng.nextFloat() * 20.0f;
        float bx = center.x + std::cos(angle) * dist;
        float by = center.y + std::sin(angle) * dist * 0.4f;
        pool.push_back({ bx, by, 2.0f + rng.nextFloat() * 3.0f, 26.0f + rng.nextFloat() * 32.0f, rng.nextFloat() * 6.28f, 0.95f });
    }
}

void Bubbles::confetti(juce::Point<float> from)
{
    const juce::uint32 colours[] = { 0xFFFF5FA2, 0xFFFFD84A, 0xFF2FD0FF, 0xFF39FF14, 0xFFFF8148, 0xFFFFFFFF };
    for (int i = 0; i < 70; ++i)
    {
        const float angle = -1.5708f + (rng.nextFloat() - 0.5f) * 2.6f;
        const float speed = 160.0f + rng.nextFloat() * 260.0f;
        confettiPool.push_back({ from.x, from.y, std::cos(angle) * speed, std::sin(angle) * speed,
                                 rng.nextFloat() * 6.28f, 1.0f, juce::Colour(colours[i % 6]) });
    }
}

void Bubbles::paint(juce::Graphics& g)
{
    for (const auto& c : confettiPool)
    {
        // Pixel confetti: a square that flickers thin as it tumbles.
        const float w = 3.0f + 3.0f * std::abs(std::cos(c.spin));
        g.setColour(c.colour.withAlpha(juce::jlimit(0.0f, 1.0f, c.life * 1.5f)));
        g.fillRect(std::round(c.x - w * 0.5f), std::round(c.y - 3.0f), w, 6.0f);
    }
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
