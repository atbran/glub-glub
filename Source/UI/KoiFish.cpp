#include "KoiFish.h"
#include <cmath>

KoiFish::KoiFish() {}

void KoiFish::setVibe(float e, float b, float p, float bp, int inten, int bar)
{
    energy = e; brightness = b; pulse = p; beatPhase = bp; intensity = inten;
    time += 0.016;
    if (bar >= 0 && bar != lastBarCount)
    {
        lastBarCount = bar;
        spinVelocity = 0.55f;
    }
    spinAngle += spinVelocity;
    spinVelocity *= 0.88f;
    // flip direction occasionally on high energy for cuteness
    if (pulse > 0.85f && ((int) (time * 10) % 120 == 0))
        facingRight = !facingRight;
    repaint();
}

void KoiFish::drawPixel(juce::Graphics& g, int gx, int gy, int pixel, juce::Colour c, float ox, float oy) const
{
    g.setColour(c);
    g.fillRect(ox + gx * (float) pixel, oy + gy * (float) pixel, (float) pixel, (float) pixel);
}

juce::Point<float> KoiFish::getMouthPosition() const { return mouthPos; }

void KoiFish::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth(), h = bounds.getHeight();
    if (w <= 0 || h <= 0) return;

    // Dance offsets
    float bounce = 0.0f;
    if (beatPhase >= 0.0f)
        bounce = -std::abs(std::sin(beatPhase * 3.14159f)) * (10.0f + energy * 26.0f);
    else
        bounce = std::sin(time * (2.0 + energy * 6.0)) * (3.0f + energy * 10.0f) - pulse * 8.0f;

    float sway = std::sin(time * (1.5 + energy * 4.0)) * (4.0f + energy * 10.0f);
    float wiggle = std::sin(time * (6.0 + energy * 10.0 + pulse * 6.0));

    float cx = w * 0.5f + sway;
    float cy = h * 0.46f + bounce;

    juce::Graphics::ScopedSaveState fishState(g);
    g.addTransform(juce::AffineTransform::rotation(std::sin(spinAngle) * 0.48f, cx, cy));

    int pixel = juce::jmax(4, (int) (juce::jmin(w, h) / 42));
    // 16x10 koi sprite grid (procedural, orange-red with white patches)
    static const char* rows[] = {
        "................",
        ".....XXXX.......",
        "....XXXXXXWW....",
        "...XXXXXXXXWW...",
        "..XXXXXXXXXWWW..",
        "..XXXXKXXXXWWW..",
        "...XXXXXXXXWW...",
        "....XXXXXXWW....",
        ".....XXXX.......",
        "................",
    };
    auto orange = juce::Colour(0xFFFF4A1F);
    auto darkOrange = juce::Colour(0xFFE63B12);
    auto lightOrange = juce::Colour(0xFFFF8C42);
    auto white = juce::Colour(0xFFFFF3E8);
    auto black = juce::Colour(0xFF1A1A1A);

    float ox = cx - 8 * pixel;
    float oy = cy - 5 * pixel;

    // tail (wagging triangles of pixels)
    int wag = (int) (wiggle * 1.5f);
    g.setColour(lightOrange);
    for (int i = 0; i < 3; ++i)
    {
        int tx = facingRight ? -2 - i : 16 + i;
        int ty = 4 + wag * (i % 2 == 0 ? 1 : -1) / 2 + (i - 1);
        g.fillRect(ox + tx * pixel, oy + ty * pixel, (float) pixel * 1.6f, (float) pixel * 1.6f);
    }

    for (int gy = 0; gy < 10; ++gy)
    {
        const char* row = rows[gy];
        for (int gx = 0; gx < 16; ++gx)
        {
            char c = row[gx];
            if (c == '.') continue;
            int sx = facingRight ? gx : (15 - gx);
            juce::Colour col = orange;
            if (c == 'W') col = white;
            else if (c == 'K') col = black;
            else if ((gx + gy) % 3 == 0) col = darkOrange;
            // sparkle tint on high brightness
            if (intensity == 2 && (gx * 7 + gy * 13) % 5 == 0)
                col = col.brighter(0.35f);
            drawPixel(g, sx, gy + (int) (wiggle * 0.4f * (gx - 8) / 8.0f), pixel, col, ox, oy);
        }
    }

    // fins (bounce with pulse)
    g.setColour(darkOrange);
    float finY = oy + 6 * pixel + pulse * 3.0f;
    g.fillRect(ox + 6 * pixel, finY, (float) pixel * 2.4f, (float) pixel);

    // mouth pos (front center)
    float mx = facingRight ? ox + 16 * pixel : ox;
    mouthPos = { mx, oy + 5 * pixel };

    // blush + sparkles when hype
    if (intensity == 2)
    {
        g.setColour(juce::Colour(0xFFFFD166));
        for (int i = 0; i < 4; ++i)
        {
            float sx = cx + std::cos(time * 3.0 + i * 1.7) * w * 0.28f;
            float sy = cy + std::sin(time * 2.4 + i * 2.1) * h * 0.26f;
            g.fillRect(sx, sy, (float) pixel * 0.7f, (float) pixel * 0.7f);
        }
    }
}
