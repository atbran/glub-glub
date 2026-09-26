#include "TankScene.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kPi = 3.14159265f;

    unsigned hashCell(int x, int y)
    {
        unsigned h = (unsigned) (x * 374761393 + y * 668265263);
        h = (h ^ (h >> 13)) * 1274126177u;
        return h ^ (h >> 16);
    }

    const char* const kChestArt[9] = {
        "..OOOOOOOOOO..",
        ".OBBBBBBBBBBO.",
        "OBBHBBBBBBHBBO",
        "OOOOOOOOOOOOOO",
        "OWWWWWGGWWWWWO",
        "OWWDWWGGWWWDWO",
        "OWWWWWWWWWWWWO",
        "ODWWWWWWWWWWDO",
        "OOOOOOOOOOOOOO",
    };

    const char* const kCoralArt[10] = {
        "..L...L..L.",
        "..C.L.C..C.",
        ".CC.C.CC.C.",
        ".C..CC.C.CC",
        ".CC..C.CC..",
        "..CC.C.C...",
        "...CCCCC...",
        "....CCC....",
        ".....C.....",
        ".....C.....",
    };

    // Neon tetra, facing right: blue stripe over a red belly.
    const char* const kTetraArt[3] = {
        ".BBBB.",
        "TRRRBE",
        ".RRR..",
    };
}

TankScene::TankScene()
{
    setInterceptsMouseClicks(false, false);
    setOpaque(true);
}

TankScene::Palette TankScene::paletteFor(Theme t, float hueShift)
{
    auto hsv = [hueShift](float h, float s, float v) { return juce::Colour::fromHSV(std::fmod(h + hueShift + 1.0f, 1.0f), s, v, 1.0f); };
    Palette p;
    switch (t)
    {
        case Theme::Midnight:
            p = { hsv(0.64f, 0.70f, 0.24f), hsv(0.70f, 0.80f, 0.05f),
                  juce::Colour(0xFF3A3552), juce::Colour(0xFF26223A), juce::Colour(0xFF4F4870),
                  juce::Colour(0xFF1E2233), juce::Colour(0xFF14172A),
                  juce::Colour(0xFF1F6B5C), juce::Colour(0xFF4DFFE1),
                  juce::Colour(0xFFB23AA8), juce::Colour(0xFFFF7FF0),
                  juce::Colour(0xFFB9C8FF), juce::Colour(0xFF4DFFE1), juce::Colour(0xFF8CFFF0), 1.0f };
            break;
        case Theme::Sunset:
            p = { hsv(0.03f, 0.50f, 0.78f), hsv(0.83f, 0.55f, 0.24f),
                  juce::Colour(0xFFE0A872), juce::Colour(0xFFB57E4E), juce::Colour(0xFFF5CC98),
                  juce::Colour(0xFF6B4A5E), juce::Colour(0xFF4A3244),
                  juce::Colour(0xFF4F8A3A), juce::Colour(0xFF8CC063),
                  juce::Colour(0xFFFF6F61), juce::Colour(0xFFFFD166),
                  juce::Colour(0xFFFFE3A3), juce::Colour(0xFFFFB067), juce::Colour(0xFFFFF1D6), 0.25f };
            break;
        case Theme::Lagoon:
        default:
            p = { hsv(0.53f, 0.60f, 0.46f), hsv(0.59f, 0.65f, 0.13f),
                  juce::Colour(0xFFC9A66B), juce::Colour(0xFF9C7A45), juce::Colour(0xFFE3C58E),
                  juce::Colour(0xFF3B4A57), juce::Colour(0xFF27323D),
                  juce::Colour(0xFF2E8B57), juce::Colour(0xFF5CC47F),
                  juce::Colour(0xFFFF7F6B), juce::Colour(0xFFFFB3A1),
                  juce::Colour(0xFFE6FFFF), juce::Colour(0xFF9FF3FF), juce::Colour(0xFFDDF8FF), 0.35f };
            break;
    }
    return p;
}

void TankScene::setTheme(Theme t, float hueShift)
{
    if (t == theme && std::abs(hueShift - hue) < 0.0005f && cache.isValid()) return;
    theme = t;
    hue = hueShift;
    pal = paletteFor(t, hueShift);
    cache = {};
    repaint();
}

void TankScene::renderStatic()
{
    const int w = getWidth(), h = getHeight();
    if (w <= 0 || h <= 0) return;
    cell = juce::jmax(3, w / 125);
    cache = juce::Image(juce::Image::RGB, w, h, false, juce::SoftwareImageType());
    juce::Graphics g(cache);
    const float c = (float) cell;
    const int cols = w / cell + 1, rows = h / cell + 1;

    // Water: quantised gradient bands with a checker dither at each seam.
    const int bands = 16;
    for (int y = 0; y < rows; ++y)
    {
        const float t = juce::jlimit(0.0f, 1.0f, (float) (y * cell) / juce::jmax(1.0f, floorY));
        const float scaled = t * (bands - 1);
        const int band = (int) scaled;
        const float frac = scaled - (float) band;
        const auto a = pal.waterTop.interpolatedWith(pal.waterBottom, (float) band / (bands - 1));
        const auto b = pal.waterTop.interpolatedWith(pal.waterBottom, (float) juce::jmin(bands - 1, band + 1) / (bands - 1));
        for (int x = 0; x < cols; ++x)
        {
            const bool dither = frac > 0.66f && ((x + y) & 1) == 0;
            g.setColour(dither ? b : a);
            g.fillRect(x * c, y * c, c, c);
        }
    }

    auto sandTop = [&](int x) { return floorY - 2.0f * c + std::sin(x * 0.11f) * 1.5f * c + std::sin(x * 0.037f + 1.0f) * 2.0f * c; };

    // Distant rock mounds, half lost in the water.
    const float mounds[4][3] = { { 0.18f, 0.16f, 0.10f }, { 0.47f, 0.22f, 0.07f }, { 0.78f, 0.18f, 0.12f }, { 0.97f, 0.12f, 0.08f } };
    for (const auto& m : mounds)
    {
        const float mx = m[0] * w, rx = m[1] * w, ry = m[2] * h;
        for (int x = (int) ((mx - rx) / c); x <= (int) ((mx + rx) / c); ++x)
            for (int y = (int) ((floorY - ry) / c); y < (int) (floorY / c) + 2; ++y)
            {
                const float dx = (x * c - mx) / rx, dy = (y * c - floorY) / ry;
                if (dx * dx + dy * dy > 1.0f) continue;
                const auto water = pal.waterTop.interpolatedWith(pal.waterBottom, juce::jlimit(0.0f, 1.0f, y * c / floorY));
                g.setColour(water.interpolatedWith(pal.rockDark, dy < -0.7f ? 0.35f : 0.5f));
                g.fillRect(x * c, y * c, c, c);
            }
    }

    // Sand bed with speckles.
    for (int x = 0; x < cols; ++x)
    {
        const int top = (int) (sandTop(x) / c);
        for (int y = top; y < rows; ++y)
        {
            const unsigned hsh = hashCell(x, y);
            juce::Colour col = pal.sand;
            if (y == top) col = pal.sandLight;
            else if (hsh % 9 == 0) col = pal.sandDark;
            else if (hsh % 13 == 0) col = pal.sandLight;
            else if (y > top + 1 && ((hsh >> 8) % 5 == 0)) col = pal.sand.interpolatedWith(pal.sandDark, 0.35f);
            g.setColour(col);
            g.fillRect(x * c, y * c, c, c);
        }
    }

    // Pebbles.
    juce::Random pebbleRng(11);
    for (int i = 0; i < 14; ++i)
    {
        const int px = pebbleRng.nextInt(cols);
        const int py = (int) (sandTop(px) / c) + 1 + pebbleRng.nextInt(juce::jmax(1, (int) ((h - sandTop(px)) / c) - 2));
        const int pw = 2 + pebbleRng.nextInt(2);
        g.setColour(i % 3 == 0 ? pal.rock : pal.rockDark);
        g.fillRect(px * c, py * c, pw * c, c);
        g.fillRect((px + 1) * c, (py - 1) * c, (pw - 2 + 1) * c, c);
    }

    // Coral: left of centre, on the sand.
    {
        const int cx = (int) (0.30f * w / c) - 5;
        const int baseY = (int) (sandTop(cx + 5) / c) + 1 - 10;
        for (int r = 0; r < 10; ++r)
            for (int x = 0; x < 11; ++x)
            {
                const char ch = kCoralArt[r][x];
                if (ch == '.') continue;
                g.setColour(ch == 'L' ? pal.coralLight : pal.coral);
                g.fillRect((cx + x) * c, (baseY + r) * c, c, c);
            }
        coralCentre = { (cx + 5.5f) * c, (baseY + 4.0f) * c };
    }

    // Sunken treasure chest.
    {
        const int cx = (int) (0.64f * w / c) - 7;
        const int baseY = (int) (sandTop(cx + 7) / c) - 6;
        for (int r = 0; r < 9; ++r)
            for (int x = 0; x < 14; ++x)
            {
                juce::Colour col;
                switch (kChestArt[r][x])
                {
                    case 'O': col = juce::Colour(0xFF2A170C); break;
                    case 'B': col = juce::Colour(0xFF8B5A2B); break;
                    case 'H': col = juce::Colour(0xFFB07B45); break;
                    case 'W': col = juce::Colour(0xFF6E4424); break;
                    case 'D': col = juce::Colour(0xFF5A351B); break;
                    case 'G': col = juce::Colour(0xFFFFC93C); break;
                    default: continue;
                }
                g.setColour(col);
                g.fillRect((cx + x) * c, (baseY + r) * c, c, c);
            }
        chestMouth = { (cx + 7.0f) * c, (baseY + 1.0f) * c };
    }

    // Seaweed roots: rocks at the base of each clump.
    for (const float fx : { 0.09f, 0.80f })
    {
        const int rx = (int) (fx * w / c);
        const int ry = (int) (sandTop(rx) / c);
        g.setColour(pal.rock);
        g.fillRect((rx - 3) * c, ry * c, 7 * c, 2 * c);
        g.fillRect((rx - 2) * c, (ry - 1) * c, 5 * c, c);
        g.setColour(pal.rockDark);
        g.fillRect((rx - 3) * c, (ry + 1) * c, 7 * c, c);
    }
}

void TankScene::addRipple(juce::Point<float> p)
{
    ripples.push_back({ p.x, p.y, 2.0f, 0.9f });
}

bool TankScene::consumeChestBurp()
{
    const bool b = chestBurp;
    chestBurp = false;
    return b;
}

void TankScene::update(const Frame& f, double nowSec, float dt)
{
    frame = f;
    time = nowSec;
    weedKick = juce::jmax(weedKick * std::exp(-dt / 0.25f), f.kick);

    // Every eighth kick the chest lid rattles loose a bubble.
    if (f.kick > 0.85f && !kickHeld)
    {
        kickHeld = true;
        if (++kickCount % 8 == 0) chestBurp = true;
    }
    else if (f.kick < 0.5f) kickHeld = false;

    for (auto& rip : ripples)
    {
        rip.radius += 108.0f * dt;
        rip.alpha -= 1.9f * dt;
    }
    ripples.erase(std::remove_if(ripples.begin(), ripples.end(),
        [](const Ripple& r) { return r.alpha <= 0.0f || r.radius > 70.0f; }), ripples.end());

    const float w = (float) getWidth();
    for (auto& s : specks)
    {
        if (s.speed <= 0.0f)
            s = { rng.nextFloat() * w, rng.nextFloat() * floorY, 3.0f + rng.nextFloat() * 7.0f, rng.nextFloat() * 6.28f };
        s.y -= s.speed * dt;
        s.x += std::sin((float) time * 0.4f + s.phase) * 4.0f * dt;
        if (s.y < 0.0f) { s.y = floorY; s.x = rng.nextFloat() * w; }
    }

    updateTetras(dt);
    repaint();
}

void TankScene::updateTetras(float dt)
{
    const float w = (float) getWidth();
    if (w <= 0.0f) return;
    const float top = floorY * 0.12f, bottom = floorY * 0.85f;
    if (!tetrasPlaced)
    {
        for (size_t i = 0; i < tetras.size(); ++i)
            tetras[i] = { { w * 0.2f + (float) i * 9.0f, floorY * 0.3f + (float) (i % 3) * 8.0f }, { 20.0f, 0.0f }, (float) i };
        schoolGoal = { w * 0.7f, floorY * 0.35f };
        tetrasPlaced = true;
    }

    goalTimer -= dt;
    dartCooldown = juce::jmax(0.0f, dartCooldown - dt);
    juce::Point<float> centre;
    for (const auto& t : tetras) centre += t.pos;
    centre /= (float) tetras.size();
    if (goalTimer <= 0.0f || centre.getDistanceFrom(schoolGoal) < 25.0f)
    {
        schoolGoal = { w * (0.1f + 0.8f * rng.nextFloat()), top + (bottom - top) * rng.nextFloat() };
        goalTimer = 4.0f + rng.nextFloat() * 4.0f;
    }

    const bool dart = frame.kick > 0.85f && dartCooldown <= 0.0f;
    if (dart) dartCooldown = 1.4f;

    for (size_t i = 0; i < tetras.size(); ++i)
    {
        auto& t = tetras[i];
        t.phase += dt * 9.0f;
        const juce::Point<float> slot { std::cos((float) i * 2.4f) * 16.0f, std::sin((float) i * 2.4f) * 9.0f };
        auto steer = (schoolGoal + slot - t.pos) * 0.9f;
        for (const auto& o : tetras)
        {
            const auto d = t.pos - o.pos;
            const float dist = d.getDistanceFromOrigin();
            if (dist > 0.01f && dist < 12.0f) steer += d / dist * (12.0f - dist) * 12.0f;
        }
        const auto away = t.pos - frame.koi;
        const float koiDist = away.getDistanceFromOrigin();
        if (koiDist < 110.0f && koiDist > 0.01f) steer += away / koiDist * (110.0f - koiDist) * 5.0f;
        if (dart && koiDist > 0.01f)
            t.vel += (away / koiDist) * 150.0f + juce::Point<float>(0.0f, -40.0f);
        t.vel += steer * dt;
        t.vel *= std::exp(-dt * 0.9f);
        const float speed = t.vel.getDistanceFromOrigin();
        const float maxSpeed = 70.0f + 160.0f * juce::jlimit(0.0f, 1.0f, dartCooldown - 0.4f);
        if (speed > maxSpeed) t.vel *= maxSpeed / speed;
        t.pos += t.vel * dt;
        t.pos.x = juce::jlimit(6.0f, w - 6.0f, t.pos.x);
        t.pos.y = juce::jlimit(top, bottom, t.pos.y);
    }
}

void TankScene::drawSeaweed(juce::Graphics& g, float x, float baseY, int segments, float phase, bool front)
{
    const float c = (float) cell;
    const float amp = (1.0f + 2.2f * frame.low + 3.2f * weedKick) * c;
    auto dark = front ? pal.weed : pal.weed.interpolatedWith(pal.waterBottom, 0.35f);
    auto light = front ? pal.weedLight : pal.weedLight.interpolatedWith(pal.waterBottom, 0.35f);
    const float glow = frame.low * (0.08f + 0.40f * pal.glowBoost);
    for (int s = 0; s < segments; ++s)
    {
        const float t = (float) s / (float) segments;
        const float sway = std::sin((float) time * 1.3f + phase - (float) s * 0.33f) * amp * std::pow(t, 1.4f);
        const float sx = std::round((x + sway) / c) * c;
        const float sy = baseY - (float) (s + 1) * c;
        const int width = s < segments * 0.7f ? 2 : 1;
        if (glow > 0.02f)
        {
            g.setColour(pal.glow.withAlpha(glow * (0.4f + 0.6f * t)));
            g.fillRect(sx - c, sy, (width + 2) * c, c);
        }
        g.setColour(dark);
        g.fillRect(sx, sy, width * c, c);
        if (width == 2 && (s % 3) != 1)
        {
            g.setColour(light);
            g.fillRect(sx + ((s / 3) % 2 == 0 ? 0.0f : c), sy, c, c);
        }
        if (s % 5 == 3) // leaf nubs
        {
            g.setColour(light);
            g.fillRect(sx + ((s / 5) % 2 == 0 ? -c : width * c), sy, c, c);
        }
    }
}

void TankScene::paint(juce::Graphics& g)
{
    if (!cache.isValid()) renderStatic();
    if (!cache.isValid()) return;
    g.drawImageAt(cache, 0, 0);

    const float w = (float) getWidth(), h = (float) getHeight(), c = (float) cell;
    const bool night = theme == Theme::Midnight;

    // Light rays from the surface; the treble makes them shimmer.
    for (int i = 0; i < 4; ++i)
    {
        const float x0 = w * (0.12f + 0.26f * i) + std::sin((float) time * 0.13f + i * 1.7f) * w * 0.06f;
        const float slant = w * 0.16f;
        const float topW = 14.0f + 8.0f * (float) (i % 3), botW = 60.0f + 14.0f * (float) (i % 2);
        const float alpha = (0.035f + 0.09f * frame.high + 0.025f * std::sin((float) time * 0.7f + i)) * (night ? 0.7f : 1.0f);
        if (alpha <= 0.0f) continue;
        juce::Path ray;
        ray.startNewSubPath(x0, 0);
        ray.lineTo(x0 + topW, 0);
        ray.lineTo(x0 + slant + botW, floorY);
        ray.lineTo(x0 + slant, floorY);
        ray.closeSubPath();
        g.setGradientFill(juce::ColourGradient(pal.ray.withAlpha(alpha), x0, 0, pal.ray.withAlpha(0.0f), x0 + slant, floorY, false));
        g.fillPath(ray);
    }

    // Plankton: drifts up, twinkles with the highs (and glows at night).
    for (const auto& s : specks)
    {
        const float tw = 0.5f + 0.5f * std::sin((float) time * 2.0f + s.phase);
        const float a = juce::jlimit(0.0f, 1.0f, (0.10f + 0.55f * frame.high * tw) * (1.0f + pal.glowBoost));
        const float sx = std::round(s.x / c) * c, sy = std::round(s.y / c) * c;
        if (night)
        {
            g.setColour(pal.speck.withAlpha(a * 0.25f));
            g.fillRect(sx - c, sy - c, 3 * c, 3 * c);
        }
        g.setColour(pal.speck.withAlpha(a));
        g.fillRect(sx, sy, c * 0.75f, c * 0.75f);
    }

    // Coral glows with the mids.
    if (frame.mid > 0.02f)
    {
        const float r = 16.0f * c;
        const float a = frame.mid * (0.10f + 0.30f * pal.glowBoost);
        g.setGradientFill(juce::ColourGradient(pal.coralLight.withAlpha(a), coralCentre.x, coralCentre.y,
                                               pal.coralLight.withAlpha(0.0f), coralCentre.x + r, coralCentre.y, true));
        g.fillEllipse(coralCentre.x - r, coralCentre.y - r, r * 2, r * 2);
    }

    // Seaweed pumps with the bass and snaps on kicks.
    const float weedBase = floorY - c;
    drawSeaweed(g, w * 0.06f, weedBase, (int) (floorY * 0.34f / c), 0.0f, false);
    drawSeaweed(g, w * 0.83f, weedBase, (int) (floorY * 0.28f / c), 2.1f, false);
    drawSeaweed(g, w * 0.09f, weedBase + c, (int) (floorY * 0.42f / c), 0.9f, true);
    drawSeaweed(g, w * 0.12f, weedBase + c, (int) (floorY * 0.26f / c), 1.8f, true);
    drawSeaweed(g, w * 0.80f, weedBase + c, (int) (floorY * 0.36f / c), 3.0f, true);

    // Tank mates.
    if (frame.tankMates)
    {
        for (const auto& t : tetras)
        {
            const bool right = t.vel.x >= 0.0f;
            const float tc = juce::jmax(2.0f, c * 0.75f);
            const float ox = std::round(t.pos.x - 3.0f * tc);
            const float oy = std::round(t.pos.y - 1.5f * tc + std::sin(t.phase) * 0.6f);
            for (int r = 0; r < 3; ++r)
                for (int x = 0; x < 6; ++x)
                {
                    const char ch = kTetraArt[r][right ? x : 5 - x];
                    juce::Colour col;
                    switch (ch)
                    {
                        case 'B': col = night ? juce::Colour(0xFF4DFFE1) : juce::Colour(0xFF2FD0FF); break;
                        case 'R': col = juce::Colour(0xFFE8412C); break;
                        case 'T': col = juce::Colour(0xFFBFD8E0).withAlpha(0.7f); break;
                        case 'E': col = juce::Colour(0xFF101418); break;
                        default: continue;
                    }
                    g.setColour(col);
                    g.fillRect(ox + x * tc, oy + r * tc, tc, tc);
                }
        }
    }

    // Disco lasers sweep from the ball when the party is on.
    if (frame.discoAppear > 0.05f)
    {
        const float baseAlpha = frame.discoAppear * (0.35f + 0.35f * frame.pulse);
        struct LaserDef { juce::Colour col; float freq, phase, spread; };
        const LaserDef lasers[4] = {
            { juce::Colour(0xFF00E5FF), 2.2f, 0.0f, 0.55f },
            { juce::Colour(0xFFFF007F), 1.7f, 1.3f, 0.65f },
            { juce::Colour(0xFF39FF14), 2.8f, 2.7f, 0.50f },
            { juce::Colour(0xFFFFD700), 1.9f, 4.2f, 0.70f },
        };
        const auto o = frame.discoCentre;
        for (const auto& l : lasers)
        {
            const float ang = 0.5f * kPi + l.spread * std::sin((float) time * l.freq + l.phase);
            const float length = h * 1.5f;
            const float ex = o.x + std::cos(ang) * length, ey = o.y + std::sin(ang) * length;
            g.setColour(l.col.withAlpha(baseAlpha * 0.25f));
            g.drawLine(o.x, o.y, ex, ey, 6.0f);
            g.setColour(l.col.withAlpha(baseAlpha * 0.75f));
            g.drawLine(o.x, o.y, ex, ey, 2.0f);
            const float tFloor = (floorY - o.y) / (std::sin(ang) * length);
            if (tFloor > 0.0f && tFloor < 1.0f)
            {
                const float hx = o.x + std::cos(ang) * length * tFloor;
                g.setColour(l.col.withAlpha(baseAlpha * 0.6f));
                g.fillEllipse(hx - 8.0f, floorY - 4.0f, 16.0f, 8.0f);
            }
        }
    }

    // Ripples from tapping the glass.
    for (const auto& rip : ripples)
    {
        g.setColour(juce::Colour(0xFFBEE9E8).withAlpha(rip.alpha * 0.75f));
        g.drawEllipse(rip.x - rip.radius, rip.y - rip.radius * 0.45f, rip.radius * 2.0f, rip.radius * 0.9f, 2.0f);
        if (rip.radius > 8.0f)
        {
            const float r2 = rip.radius - 6.0f;
            g.setColour(juce::Colours::white.withAlpha(rip.alpha * 0.4f));
            g.drawEllipse(rip.x - r2, rip.y - r2 * 0.45f, r2 * 2.0f, r2 * 0.9f, 1.0f);
        }
    }

    if (frame.partyGlow > 0.004f)
    {
        const float a = juce::jlimit(0.0f, 0.14f, (0.08f + 0.05f * frame.pulse) * frame.partyGlow);
        g.setColour(juce::Colour::fromHSV(frame.partyHue, 0.55f, 0.9f, a));
        g.fillAll();
    }
}
