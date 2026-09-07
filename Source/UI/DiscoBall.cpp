#include "DiscoBall.h"
#include <cmath>

namespace
{
    constexpr float kPi = 3.14159265f;
}

DiscoBall::DiscoBall()
{
    setInterceptsMouseClicks(false, false);
}

void DiscoBall::update(float hypeLevel, double nowSec)
{
    float dt = (float) juce::jlimit(0.0, 0.1, nowSec - lastNow);
    lastNow = nowSec;
    time = nowSec;
    level = hypeLevel;

    if (!showing)
    {
        aboveTime = level > 0.45f ? aboveTime + dt : 0.0f;
        if (aboveTime >= 0.10f)
        {
            showing = true;
            shownAt = nowSec;
            aboveTime = 0.0f;
            belowTime = 0.0f;
        }
    }
    else
    {
        belowTime = level < 0.60f ? belowTime + dt : 0.0f;
        if (belowTime >= 0.50f)
        {
            showing = false;
            belowTime = 0.0f;
            aboveTime = 0.0f;
        }
    }

    appear = juce::jlimit(0.0f, 1.0f, appear + (showing ? 0.026f : -0.05f));

    if (appear > 0.001f)
        repaint();
}

void DiscoBall::paint(juce::Graphics& g)
{
    if (appear <= 0.001f) return;

    auto b = getLocalBounds().toFloat();
    float anchorX = b.getWidth() * 0.5f;

    float t = juce::jlimit(0.0f, 1.0f, appear);
    float c1 = 1.70158f, c3 = c1 + 1.0f;
    float eased = 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
    float drop = (1.0f - eased) * -90.0f;

    int cell = juce::jmax(3, (int) (b.getHeight() / 26.0f));
    float cf = (float) cell;

    g.setColour(juce::Colour(0xFF9AA5AE));
    for (int i = 0; i < 4; ++i)
        g.fillRect(anchorX - cf * 0.5f, (float) (i * cell) + drop, cf, cf);

    float bx = anchorX - 5.5f * cf;
    float by = 4.0f * cf + drop;
    float bcx = bx + 5.5f * cf;
    float bcy = by + 5.5f * cf;
    const int R = 5;
    const int span = 2 * R + 1;

    int shift = (int) (time * 1.2f);
    float specX = std::fmod((float) time * 1.7f, (float) (span + 6)) - 3.0f;

    for (int dy = -R; dy <= R; ++dy)
    {
        for (int dx = -R; dx <= R; ++dx)
        {
            float fx = (float) dx, fy = (float) dy;
            float d2 = fx * fx + fy * fy;
            if (d2 > (float) (R * R)) continue;

            int cx2 = ((dx + shift) % span + span) % span - R;

            int parity = ((cx2 + dy) & 1);
            float diag = ((float) (cx2 + dy) / (float) (2 * R) + 1.0f) * 0.5f;
            float hue = 0.58f + diag * 0.34f;

            float latB = 1.0f - std::abs((float) dy) / (float) R;
            float bright = 0.46f + 0.24f * latB + (parity != 0 ? 0.10f : -0.06f);
            bright = std::round(bright * 4.0f) / 4.0f;

            unsigned hsh = (unsigned) ((cx2 * 73 + dy * 151) & 0x7fffffff);
            float specD = std::abs((float) cx2 - specX);
            bool interior = d2 <= (float) ((R - 1) * (R - 1));

            if (interior && specD < 1.2f)
                g.setColour(juce::Colour(0xFFFFFFFF));
            else if (interior && specD < 2.2f)
                g.setColour(juce::Colour::fromHSV(hue, 0.30f, 1.0f, 1.0f));
            else if (!interior)
                g.setColour(juce::Colour(0xFF4A4F57));
            else if (hsh % 17 == 0)
                g.setColour(juce::Colour(0xFFF7F3FF));
            else
                g.setColour(juce::Colour::fromHSV(hue, 0.45f, juce::jlimit(0.30f, 0.95f, bright), 1.0f));

            g.fillRect(bx + (float) (dx + R) * cf, by + (float) (dy + R) * cf, cf, cf);
        }
    }

    for (int i = 0; i < 6; ++i)
    {
        float tw = 0.5f + 0.5f * std::sin((float) time * 2.2f + i * 1.7f);
        if (tw < 0.25f) continue;
        float ang = (float) i * (2.0f * kPi / 6.0f) + (float) time * 0.2f;
        float px = bcx + std::cos(ang) * 6.6f * cf;
        float py = bcy + std::sin(ang) * 6.2f * cf;
        float s = cf * (0.7f + 0.5f * tw);
        g.setColour((i % 2 == 0 ? juce::Colour(0xFFF7F3FF) : juce::Colour(0xFFFF9EC4))
                        .withAlpha(0.25f + 0.6f * tw));
        g.fillRect(px - s, py - s * 0.20f, s * 2.0f, s * 0.40f);
        g.fillRect(px - s * 0.20f, py - s, s * 0.40f, s * 2.0f);
    }
}
