#include "KoiFish.h"
#include <cmath>

namespace
{
    // Kohaku pixel palette: white body, orange-red patches, black speckles, amber fins.
    const auto outline = juce::Colour(0xFF160D0A);
    const auto white = juce::Colour(0xFFFFF5DF);
    const auto red = juce::Colour(0xFFD84018);
    const auto black = juce::Colour(0xFF1A1A1A);
    const auto fin = juce::Colour(0xFFCFA038);

    bool inBodyEllipse(int gx, int gy)
    {
        const double rx = 12.5, ry = 6.4, cx = 19.5, cy = 11.0;
        double dx = (gx + 0.5 - cx) / rx;
        double dy = (gy + 0.5 - cy) / ry;
        return dx * dx + dy * dy <= 1.0;
    }

    bool inUpperTail(int gx, int gy)
    {
        if (gx < 0 || gx > 7) return false;
        double top = 2.0 + gx;
        double bottom = 10.0 - gx * 0.6;
        return gy + 0.5 >= top && gy + 0.5 <= bottom;
    }

    bool inLowerTail(int gx, int gy)
    {
        if (gx < 0 || gx > 7) return false;
        double top = 11.2;
        double bottom = 21.0 - gx * 1.15;
        return gy + 0.5 >= top && gy + 0.5 <= bottom;
    }

    bool inDorsalFin(int gx, int gy)
    {
        if (gx < 14 || gx > 20) return false;
        double peakY = 5.0;
        double baseY = 9.0;
        return gy + 0.5 >= peakY + std::abs(gx - 17) * 0.8 - 3.0 && gy + 0.5 <= baseY;
    }

    bool inPectoralFin(int gx, int gy)
    {
        if (gx < 22 || gx > 27) return false;
        double center = 24.5;
        double top = 15.0 + std::abs((double) gx - center) * 0.6;
        double bottom = 21.0 - std::abs((double) gx - center) * 0.2;
        return gy + 0.5 >= top && gy + 0.5 <= bottom;
    }

    bool inBarbel(int gx, int gy)
    {
        if (gx == 31) return gy == 14 || gy == 16;
        if (gx == 32) return gy == 15 || gy == 17;
        if (gx == 33) return gy == 16 || gy == 18;
        return false;
    }
}

KoiFish::KoiFish() {}

bool KoiFish::maskAt(int gx, int gy) const
{
    return inBodyEllipse(gx, gy)
        || inUpperTail(gx, gy)
        || inLowerTail(gx, gy)
        || inDorsalFin(gx, gy)
        || inPectoralFin(gx, gy)
        || inBarbel(gx, gy);
}

char KoiFish::styleAt(int gx, int gy) const
{
    if (!maskAt(gx, gy)) return '.';

    // barbels are dark
    if (inBarbel(gx, gy))
        return 'K';

    // outline: any exposed edge pixel -> black
    static const int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    for (auto& d : dirs)
    {
        int nx = gx + d[0], ny = gy + d[1];
        if (nx < 0 || nx >= SPRITE_W || ny < 0 || ny >= SPRITE_H || !maskAt(nx, ny))
            return 'B';
    }

    // eye
    if (gx == 30 && gy == 8)
        return 'K';

    // fins
    if (inDorsalFin(gx, gy) || inPectoralFin(gx, gy) || inUpperTail(gx, gy) || inLowerTail(gx, gy))
        return 'T';

    // body: kohaku red patches along upper body, white belly, sparse black speckles
    bool upperBody = gy <= 10;
    bool patch = false;
    if (inBodyEllipse(gx, gy) && upperBody)
    {
        // deterministic flowing patch bands
        float waveBand = std::sin(gx * 0.42f) + std::cos(gy * 0.55f);
        patch = waveBand > 0.65f;
    }

    unsigned hash = (unsigned) (gx * 31 + gy * 17);
    if (inBodyEllipse(gx, gy) && hash % 29 == 0)
        return 'K';

    if (patch)
        return 'R';

    return 'W';
}

float KoiFish::undulationOffset(int gx, float swimSpeed, float amp) const
{
    // traveling wave from head to tail; tail lags more than head
    float tailness = juce::jlimit(0.0f, 1.0f, 1.0f - (float) gx / (SPRITE_W - 1));
    float phase = static_cast<float> (time * swimSpeed * 2.0 * 3.14159265 - tailness * 2.1);
    return std::sin(phase) * amp * (0.20f + 0.80f * tailness);
}

void KoiFish::drawPixel(juce::Graphics& g, float sx, float sy, int pixel, juce::Colour c, float ox, float oy) const
{
    g.setColour(c);
    g.fillRect(ox + sx * (float) pixel, oy + sy * (float) pixel, (float) pixel, (float) pixel);
}

void KoiFish::setVibe(float e, float b, float p, float bp, int inten, int bar)
{
    energy = e; brightness = b; pulse = p; beatPhase = bp; intensity = inten;
    time += 0.016;

    if (bar >= 0 && bar != lastBarCount)
    {
        lastBarCount = bar;
        spinVelocity = 0.62f;
    }

    spinAngle += spinVelocity;
    spinVelocity *= 0.90f;

    // only flip on strong beats; idle remains readable
    if (pulse > 0.9f && ((int) (time * 10) % 40 == 0))
        facingRight = !facingRight;

    repaint();
}

juce::Point<float> KoiFish::getMouthPosition() const { return mouthPos; }

void KoiFish::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth(), h = bounds.getHeight();
    if (w <= 0 || h <= 0) return;

    float bounce = 0.0f;
    if (beatPhase >= 0.0f)
        bounce = -std::abs(std::sin(beatPhase * 3.14159f)) * (5.0f + energy * 12.0f);
    else
        bounce = std::sin(time * (2.0 + energy * 5.0)) * (2.5f + energy * 7.0f) - pulse * 5.0f;

    float sway = std::sin(time * (1.2 + energy * 3.0)) * (2.0f + energy * 5.0f);

    float cx = w * 0.5f + sway;
    float cy = h * 0.42f + bounce;

    juce::Graphics::ScopedSaveState state(g);
    g.addTransform(juce::AffineTransform::rotation(std::sin(spinAngle) * 0.55f, cx, cy));

    int pixel = juce::jmax(4, (int) (juce::jmin(w, h) / (SPRITE_W + 12)));
    float ox = cx - SPRITE_W * 0.5f * pixel;
    float oy = cy - SPRITE_H * 0.5f * pixel;

    float swimSpeed = 0.8f + energy * 3.2f;
    float amp = 0.35f + energy * 1.4f + pulse * 0.35f;

    for (int gy = 0; gy < SPRITE_H; ++gy)
    {
        for (int gx = 0; gx < SPRITE_W; ++gx)
        {
            char c = styleAt(gx, gy);
            if (c == '.') continue;

            int sx = facingRight ? gx : (SPRITE_W - 1 - gx);
            float dy = undulationOffset(gx, swimSpeed, amp);
            juce::Colour col = outline;
            if (c == 'W') col = white;
            else if (c == 'R') col = red;
            else if (c == 'K') col = black;
            else if (c == 'T') col = fin;

            if (intensity == 2 && c != 'B' && (gx + gy * 5) % 11 == 0)
                col = col.brighter(0.25f);

            drawPixel(g, (float) sx, (float) gy + dy, pixel, col, ox, oy);
        }
    }

    // mouth position uses head coordinate after the same undulation math
    int sourceGX = SPRITE_W - 1;
    int sourceGY = 10;
    float mouthScreenGX = facingRight ? (float) (SPRITE_W - 1) : 0.0f;
    float mouthDY = undulationOffset(sourceGX, swimSpeed, amp);
    mouthPos = { ox + mouthScreenGX * pixel, oy + (sourceGY + mouthDY) * pixel };

    if (intensity == 2)
    {
        g.setColour(juce::Colour(0xFFFFF3D0));
        for (int i = 0; i < 5; ++i)
        {
            float sx = cx + std::cos(time * 2.7 + i * 1.25) * w * 0.30f;
            float sy = cy + std::sin(time * 2.1 + i * 1.85) * h * 0.28f;
            g.fillRect(sx, sy, (float) pixel * 0.55f, (float) pixel * 0.55f);
        }
    }
}
