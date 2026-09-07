#include "KoiFish.h"
#include <cmath>
#include <utility>

namespace
{
    constexpr float kPi = 3.14159265f;

    const juce::Colour colOutline { 0xFF261712 };
    const juce::Colour colWBase   { 0xFFFDF6E3 };
    const juce::Colour colWLight  { 0xFFFFFEF9 };
    const juce::Colour colWDark   { 0xFFE3D2A8 };
    const juce::Colour colRBase   { 0xFFE8491D };
    const juce::Colour colRLight  { 0xFFFF8148 };
    const juce::Colour colRDark   { 0xFFAE3410 };
    const juce::Colour colFBase   { 0xFFD9A441 };
    const juce::Colour colFLight  { 0xFFF4CD74 };
    const juce::Colour colFDark   { 0xFFA5762A };
    const juce::Colour colEye     { 0xFF191411 };
    const juce::Colour colGlint   { 0xFFFFFFFF };
    const juce::Colour colBlush   { 0xFFF7A097 };
    const juce::Colour colMouth   { 0xFF46281C };
    const juce::Colour colSpot    { 0xFF33201A };
    const juce::Colour colBarbel  { 0xFF6B452E };

    juce::Colour colourFor(char c)
    {
        switch (c)
        {
            case 'O': return colOutline;
            case 'w': return colWBase;
            case '1': return colWLight;
            case '2': return colWDark;
            case 'r': return colRBase;
            case '3': return colRLight;
            case '4': return colRDark;
            case 'f': return colFBase;
            case '5': return colFLight;
            case '6': return colFDark;
            case 'e': return colEye;
            case 'g': return colGlint;
            case 'b': return colBlush;
            case 'm': return colMouth;
            case 's': return colSpot;
            case 'k': return colBarbel;
            default: break;
        }
        return juce::Colours::transparentBlack;
    }

    bool isFaceChar(char c)
    {
        return c == 'e' || c == 'g' || c == 'm' || c == 'b' || c == 'k';
    }

    struct SpinePt { float x, y, r, u; };
}

KoiFish::KoiFish() {}

void KoiFish::setVibe(float e, float b, float p, float bp, int inten, int bar)
{
    energy = e; brightness = b; pulse = p; beatPhase = bp; intensity = inten;
    time += 0.016;

    if (bar >= 0 && bar != lastBarCount)
    {
        lastBarCount = bar;
        if (bar % 4 == 0)
        {
            spinVelocity = 0.45f;
            tailBurst = 1.0f;
        }
    }

    spinAngle += spinVelocity;
    spinVelocity *= 0.925f;
    tailBurst *= 0.95f;

    if (energy < 0.06f && pulse < 0.12f)
        calmTime += 0.016f;
    else
        calmTime = 0.0f;
    sleepy = calmTime > 6.0f;

    if (bar > 0 && bar % 8 == 0 && bar != lastPartyBar)
    {
        lastPartyBar = bar;
        partyActive = true;
        partyT = 0.0f;
    }

    if (partyActive)
    {
        partyT += 0.016f * 1.2f;
        if (partyT >= 1.0f)
            partyActive = false;
    }

    flipCooldown = juce::jmax(0.0f, flipCooldown - 0.016f);
    if (pulse > 0.92f && energy > 0.5f && !flipActive && !partyActive && flipCooldown <= 0.0f)
    {
        flipActive = true;
        flipT = 0.0f;
        flipMid = false;
        flipCooldown = 4.0f;
    }

    if (flipActive)
    {
        flipT += 0.016f * 1.9f;
        if (!flipMid && flipT >= 0.5f)
        {
            facingRight = !facingRight;
            flipMid = true;
        }
        if (flipT >= 1.0f)
            flipActive = false;
    }

    repaint();
}

juce::Point<float> KoiFish::getMouthPosition() const { return mouthPos; }

void KoiFish::buildGrid(std::vector<char>& grid)
{
    grid.assign((size_t) GRID_W * GRID_H, '.');

    auto set = [&](int x, int y, char c)
    {
        if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return;
        grid[(size_t) y * GRID_W + x] = c;
    };
    auto get = [&](int x, int y) -> char
    {
        if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return '.';
        return grid[(size_t) y * GRID_W + x];
    };

    const int N = 22;
    std::vector<SpinePt> sp ((size_t) N);
    const float sleepScale = sleepy ? 0.45f : 1.0f;
    const float bendAmp = juce::jlimit(0.3f, 2.6f, (0.7f + 1.3f * energy + 0.5f * pulse + 0.8f * tailBurst) * sleepScale);
    const float speed = (0.8f + 0.8f * energy + 1.15f * tailBurst) * (sleepy ? 0.55f : 1.0f);
    const float phase = (float) (time * speed * 2.0 * kPi);
    const float breathe = 1.0f + 0.05f * std::sin((float) time * 1.1f) * (1.0f - energy);

    for (int i = 0; i < N; ++i)
    {
        float u = (float) i / (float) (N - 1);
        float ampU = bendAmp * (0.12f + 0.88f * (1.0f - u));
        sp[(size_t) i] = { 7.0f + u * 22.0f,
                           11.0f + ampU * std::sin(phase - u * 4.4f),
                           (1.4f + 4.2f * std::pow(u, 0.6f)) * breathe,
                           u };
    }

    auto spineAt = [&](float u) -> SpinePt
    {
        u = juce::jlimit(0.0f, 1.0f, u);
        float fi = u * (float) (N - 1);
        int i0 = (int) fi;
        int i1 = juce::jmin(N - 1, i0 + 1);
        float f = fi - (float) i0;
        auto& a = sp[(size_t) i0];
        auto& b = sp[(size_t) i1];
        return { a.x + (b.x - a.x) * f, a.y + (b.y - a.y) * f, a.r + (b.r - a.r) * f, u };
    };

    auto normalAt = [&](float u) -> std::pair<float, float>
    {
        auto p0 = spineAt(juce::jlimit(0.0f, 1.0f, u - 0.04f));
        auto p1 = spineAt(juce::jlimit(0.0f, 1.0f, u + 0.04f));
        float dx = p1.x - p0.x, dy = p1.y - p0.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 1e-4f) return { 0.0f, -1.0f };
        return { dy / len, -dx / len };
    };

    // tail fan (stamped first so the body welds the joint shut)
    {
        const SpinePt j = sp[0];
        set((int) j.x - 1, (int) j.y, 'f');
        set((int) j.x - 2, (int) j.y, 'f');

        float wag = (0.35f + 0.45f * energy + 0.9f * tailBurst) * std::sin(phase * 1.12f + 0.8f);

        auto lobe = [&](float vy)
        {
            float dx = -1.0f, dy = vy;
            float len = std::sqrt(dx * dx + dy * dy);
            dx /= len; dy /= len;
            float c = std::cos(wag), s = std::sin(wag);
            float rdx = dx * c - dy * s, rdy = dx * s + dy * c;
            float px = -rdy, py = rdx;
            const int L = 9;
            for (int st = 1; st <= L; ++st)
            {
                float fs = (float) st;
                float halfW = 0.6f + 2.4f * std::sin(kPi * 0.92f * fs / (float) L);
                float cxg = j.x + rdx * fs, cyg = j.y + rdy * fs;
                int wHalf = (int) std::round(halfW);
                for (int o = -wHalf; o <= wHalf; ++o)
                {
                    int gx = (int) std::round(cxg + px * (float) o);
                    int gy = (int) std::round(cyg + py * (float) o);
                    char cc = 'f';
                    float side = py * (float) o;
                    if (side < -0.2f) cc = '5';
                    else if (side > 0.2f) cc = '6';
                    if (st == L && std::abs(o) == wHalf) cc = '6';
                    set(gx, gy, cc);
                }
            }
        };

        lobe(-0.85f);
        lobe(0.85f);
    }

    // body discs along the bending spine
    for (int i = 0; i < N; ++i)
    {
        const SpinePt& s = sp[(size_t) i];
        int R = (int) std::ceil(s.r);
        for (int dy = -R; dy <= R; ++dy)
        {
            for (int dx = -R; dx <= R; ++dx)
            {
                float fx = (float) dx, fy = (float) dy;
                if (fx * fx + fy * fy > s.r * s.r) continue;
                int gx = (int) std::round(s.x) + dx;
                int gy = (int) std::round(s.y) + dy;
                float t = fy / s.r;
                float u = s.u;

                bool red = (u > 0.78f && t > -0.95f && t < 0.42f)
                        || (u > 0.42f && u < 0.62f && t < 0.40f)
                        || (u > 0.10f && u < 0.27f && t > -0.10f);

                char c;
                if (t > 0.55f) c = red ? '4' : '2';
                else if (t < -0.50f) c = red ? '3' : '1';
                else c = red ? 'r' : 'w';
                set(gx, gy, c);
            }
        }
    }

    // sumi spots, only over red cells
    auto spot = [&](float u, float t, float rad)
    {
        auto p = spineAt(u);
        auto n = normalAt(u);
        float cxg = p.x - n.first * (t * p.r);
        float cyg = p.y - n.second * (t * p.r);
        int R = (int) std::ceil(rad);
        for (int dy = -R; dy <= R; ++dy)
        {
            for (int dx = -R; dx <= R; ++dx)
            {
                if ((float) (dx * dx + dy * dy) > rad * rad) continue;
                int gx = (int) std::round(cxg) + dx;
                int gy = (int) std::round(cyg) + dy;
                char cur = get(gx, gy);
                if (cur == 'r' || cur == '3' || cur == '4')
                    set(gx, gy, 's');
            }
        }
    };
    spot(0.70f, -0.15f, 1.0f);
    spot(0.52f, 0.45f, 0.9f);
    spot(0.20f, 0.20f, 0.8f);

    // dorsal fin
    {
        auto p = spineAt(0.50f);
        auto n = normalAt(0.50f);
        int bx = (int) std::round(p.x + n.first * (p.r - 0.5f));
        int by = (int) std::round(p.y + n.second * (p.r - 0.5f));
        int hh = juce::jlimit(2, 4, 3 + (int) std::round(0.8f * std::sin(phase * 1.05f)));
        set(bx, by - hh, '5');
        for (int ry = by - hh + 1; ry <= by - 1; ++ry)
            for (int rx = bx - 1; rx <= bx + 1; ++rx)
                set(rx, ry, 'f');
    }

    // pectoral fin
    {
        auto p = spineAt(0.72f);
        auto n = normalAt(0.72f);
        int bx = (int) std::round(p.x - n.first * (p.r - 0.5f));
        int by = (int) std::round(p.y - n.second * (p.r - 0.5f));
        bx += (int) std::round(0.9f * pulse) + (int) std::round(0.5f * std::sin(phase * 1.3f));
        set(bx - 1, by, 'f');
        set(bx, by, 'f');
        set(bx + 1, by, 'f');
        set(bx, by + 1, 'f');
        set(bx + 1, by + 1, 'f');
        set(bx + 2, by + 1, 'f');
        set(bx + 1, by + 2, '6');
        set(bx + 2, by + 2, '6');
    }

    // face
    {
        auto p = spineAt(0.86f);
        auto n = normalAt(0.86f);
        int ex = (int) std::round(p.x + n.first * (0.35f * p.r));
        int ey = (int) std::round(p.y + n.second * (0.35f * p.r));
        if (sleepy)
        {
            set(ex - 1, ey, 'e');
            set(ex, ey, 'e');
            set(ex + 1, ey, 'e');
            set(ex + 2, ey, 'e');
        }
        else
        {
            for (int ry = -1; ry <= 1; ++ry)
                for (int rx = 0; rx <= 1; ++rx)
                    set(ex + rx, ey + ry, 'e');
            set(ex, ey - 1, 'g');
        }

        auto pm = spineAt(1.0f);
        int mx = (int) std::round(pm.x + pm.r * 0.78f);
        int my = (int) std::round(pm.y + pm.r * 0.42f);
        set(mx - 1, my, 'm');
        set(mx, my, 'm');
        set(mx + 1, my, 'k');
        set(mx + 1, my + 1, 'k');
        mouthGX = mx - 1;
        mouthGY = my;

        auto pb = spineAt(0.93f);
        int bxx = (int) std::round(pb.x);
        int byy = (int) std::round(pb.y + pb.r * 0.55f);
        set(bxx, byy, 'b');
        set(bxx + 1, byy, 'b');
    }

    // outline pass
    std::vector<char> outlined = grid;
    for (int y = 0; y < GRID_H; ++y)
    {
        for (int x = 0; x < GRID_W; ++x)
        {
            char c = grid[(size_t) y * GRID_W + x];
            if (c == '.' || isFaceChar(c)) continue;
            if (get(x + 1, y) == '.' || get(x - 1, y) == '.'
                || get(x, y + 1) == '.' || get(x, y - 1) == '.')
                outlined[(size_t) y * GRID_W + x] = 'O';
        }
    }
    grid.swap(outlined);
}

void KoiFish::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth(), h = bounds.getHeight();
    if (w <= 0 || h <= 0) return;

    std::vector<char> grid;
    buildGrid(grid);

    float flipScale = flipActive ? std::cos(kPi * flipT) : 1.0f;
    float aScale = std::max(0.12f, std::abs(flipScale));
    float squashX = aScale * (1.0f + 0.10f * pulse);
    float squashY = (1.0f + 0.22f * (1.0f - aScale)) * (1.0f - 0.15f * pulse);

    float hopScale = beatPhase >= 0.0f ? 0.25f : 1.0f;
    float hop = -(7.0f + 12.0f * energy) * pulse * pulse * hopScale
              - (flipActive ? 10.0f * std::sin(kPi * flipT) : 0.0f);

    float bob = 0.0f;
    if (beatPhase >= 0.0f)
        bob = -std::pow(1.0f - juce::jlimit(0.0f, 1.0f, beatPhase), 1.5f) * (12.0f + 21.0f * energy);
    else
        bob = std::sin(time * (1.0 + 1.5 * energy)) * (2.2f + 4.5f * energy) * (sleepy ? 0.5f : 1.0f);

    float sway = std::sin(time * (0.6 + 1.1 * energy)) * (1.2f + 2.4f * energy) * (sleepy ? 0.5f : 1.0f);
    float cx = w * 0.5f + sway;
    float cy = h * 0.52f + bob + hop;

    float spinA = std::sin(spinAngle) * 0.45f;
    if (partyActive)
    {
        float p = juce::jlimit(0.0f, 1.0f, partyT);
        float eased = p < 0.5f ? 4.0f * p * p * p : 1.0f - std::pow(-2.0f * p + 2.0f, 3.0f) * 0.5f;
        spinA += eased * 2.0f * kPi;
    }

    int pixel = juce::jmax(5, (int) (juce::jmin(w, h) / (float) (GRID_W + 8)));
    float ox = cx - GRID_W * 0.5f * pixel;
    float oy = cy - GRID_H * 0.5f * pixel;

    auto projectX = [&](float sxg) { return GRID_W * 0.5f + (sxg - GRID_W * 0.5f) * squashX; };
    auto projectY = [&](float syg) { return GRID_H * 0.5f + (syg - GRID_H * 0.5f) * squashY; };

    if (spriteImg.isNull() || spriteImg.getWidth() != (int) w || spriteImg.getHeight() != (int) h)
        spriteImg = juce::Image(juce::Image::ARGB, (int) w, (int) h, true);
    spriteImg.clear(spriteImg.getBounds());

    {
        juce::Graphics ig(spriteImg);
        for (int gy = 0; gy < GRID_H; ++gy)
        {
            for (int gx = 0; gx < GRID_W; ++gx)
            {
                char c = grid[(size_t) gy * GRID_W + gx];
                if (c == '.') continue;
                int sx = facingRight ? gx : (GRID_W - 1 - gx);
                float px = ox + std::round(projectX((float) sx)) * (float) pixel;
                float py = oy + std::round(projectY((float) gy)) * (float) pixel;
                ig.setColour(colourFor(c));
                ig.fillRect(px, py, (float) pixel, (float) pixel);
            }
        }
    }

    juce::Graphics::ScopedSaveState state(g);
    g.addTransform(juce::AffineTransform::rotation(spinA, cx, cy));
    g.drawImageAt(spriteImg, 0, 0);

    {
        int msx = facingRight ? mouthGX : (GRID_W - 1 - mouthGX);
        float px = ox + std::round(projectX((float) msx)) * (float) pixel;
        float py = oy + std::round(projectY((float) mouthGY)) * (float) pixel;
        float dxp = px - cx, dyp = py - cy;
        float ca = std::cos(spinA), sa = std::sin(spinA);
        mouthPos = { cx + dxp * ca - dyp * sa, cy + dxp * sa + dyp * ca };
    }

    if (intensity == 2)
    {
        for (int i = 0; i < 3; ++i)
        {
            float tw = 0.5f + 0.5f * std::sin((float) time * 3.2f + i * 2.3f);
            g.setColour(juce::Colour(0xFFFFE9A8).withAlpha(0.20f + 0.55f * tw));
            float px = cx + std::cos((float) time * 1.0f + i * 2.6f) * w * 0.33f;
            float py = cy + std::sin((float) time * 0.8f + i * 1.7f) * h * 0.30f;
            float s = (float) pixel * 0.9f;
            g.fillRect(px - s, py - s * 0.22f, s * 2.0f, s * 0.44f);
            g.fillRect(px - s * 0.22f, py - s, s * 0.44f, s * 2.0f);
        }
    }

    if (sleepy)
    {
        auto drawZ = [&](float zx, float zy, float zs, float alpha)
        {
            g.setColour(juce::Colour(0xFFBEE3F0).withAlpha(alpha));
            g.fillRect(zx, zy, zs * 3.0f, zs);
            g.fillRect(zx + zs, zy + zs, zs, zs);
            g.fillRect(zx, zy + 2.0f * zs, zs * 3.0f, zs);
        };
        float headX = cx + (facingRight ? 1.0f : -1.0f) * w * 0.16f;
        float rise = std::fmod((float) time * 4.0f, 18.0f);
        float alpha1 = juce::jlimit(0.0f, 1.0f, 1.0f - rise / 18.0f) * (0.55f + 0.25f * std::sin((float) time * 2.0f));
        drawZ(headX + 20.0f, cy - 70.0f - rise, (float) pixel * 0.7f, alpha1);
        float rise2 = std::fmod((float) time * 4.0f + 9.0f, 18.0f);
        float alpha2 = juce::jlimit(0.0f, 1.0f, 1.0f - rise2 / 18.0f) * 0.7f;
        drawZ(headX + 44.0f, cy - 90.0f - rise2, (float) pixel * 0.55f, alpha2);
    }
}
