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

    const char* const kDealWithItArt[4] = {
        "BBBBBBBBBBBBBBBBBBBBBBBBBBB..",
        ".BWBBBBBB..BWBWBBBBBB......BB",
        "..BWBBBB....BWBWBBBB.........",
        "...BBBB......BBBBBB.........."
    };

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

bool KoiFish::isMoveBusy() const
{
    return wormActive || rollActive || partyActive || flipActive || extraMove != MoveType::None;
}

juce::Point<float> KoiFish::figureEightPath(float progress, float width, float height)
{
    // Four eased arcs: one landmark every two beats, without shrinking either
    // lobe. Multiplying the coordinates by an envelope distorts the eight.
    const float arcs = juce::jlimit(0.0f, 1.0f, progress) * 4.0f;
    const float whole = std::floor(arcs);
    const float fraction = arcs - whole;
    const float eased = fraction * fraction * (3.0f - 2.0f * fraction);
    const float angle = (whole + eased) * 0.5f * kPi;
    return { std::sin(angle) * juce::jmin(110.0f, width * 0.22f),
             std::sin(2.0f * angle) * juce::jmin(66.0f, height * 0.16f) };
}

void KoiFish::setVibe(float e, float b, float p, float bp, int inten, int bar, float feed, float hypeLevel, float deltaSeconds, float bpm)
{
    const float dt = juce::jlimit(0.0f, 0.05f, deltaSeconds);
    const float follow = 1.0f - std::exp(-dt / 0.10f);
    energy += (e - energy) * follow;
    pulse += (p - pulse) * (1.0f - std::exp(-dt / 0.045f));
    brightness = b; beatPhase = bp; intensity = inten; hype = hypeLevel;
    time += dt;
    const float tempo = bpm > 0.0f ? juce::jlimit(40.0f, 240.0f, bpm) : 120.0f;
    freeBeat += dt * tempo / 60.0;
    danceBeat = bp >= 0.0f ? bp : (float) std::fmod(freeBeat, 1.0);
    beatStep = dt * tempo / 60.0f;
    if (bp >= 0.0f && previousBeat >= 0.0f)
    {
        float elapsed = bp - previousBeat;
        if (elapsed < -0.5f) elapsed += 1.0f;
        beatIdleTime = std::abs(elapsed) < 0.00001f ? beatIdleTime + dt : 0.0f;
        // Follow host phase changes, including tempo automation. A seek or a
        // stopped transport must not fast-forward or strand a manual move.
        if (elapsed > 0.0f && elapsed < 0.25f) beatStep = elapsed;
    }
    else beatIdleTime = 0.0f;
    previousBeat = bp;
    // Integrate changing speeds rather than multiplying elapsed time by them.
    swimPhase += bp >= 0.0f && !sleepy ? beatStep * 2.0f * kPi
        : dt * (0.8f + 0.8f * energy + 1.15f * tailBurst) * (sleepy ? 0.55f : 1.0f) * 2.0f * kPi;
    driftPhase += dt * (0.6f + 1.1f * energy);
    tailBurst *= std::exp(-dt / 0.30f);
    calmTime = energy < 0.06f && pulse < 0.12f && feed < 0.5f ? calmTime + dt : 0.0f;
    sleepy = calmTime > 6.0f;
    moveCooldown = juce::jmax(0.0f, moveCooldown - dt);
    mouseFlipCooldown = juce::jmax(0.0f, mouseFlipCooldown - dt);
    startleT = juce::jmax(0.0f, startleT - dt * 2.2f);

    auto advance = [this](bool& active, float& progress, float beats)
    {
        if (!active) return;
        progress = juce::jmin(1.0f, progress + beatStep / beats);
        if (progress >= 1.0f) active = false;
    };
    const bool wasBusy = isMoveBusy();
    advance(wormActive, wormT, 4.0f);
    advance(rollActive, rollT, 4.0f);
    advance(partyActive, partyT, 4.0f);
    if (flipActive)
    {
        flipT = juce::jmin(1.0f, flipT + beatStep);
        if (!flipMid && flipT >= 0.5f) { facingRight = !facingRight; flipMid = true; }
        if (flipT >= 1.0f) flipActive = false;
    }
    if (extraMove != MoveType::None)
    {
        const float beats = (extraMove == MoveType::FigureEight || extraMove == MoveType::Twerk) ? 8.0f : 4.0f;
        extraT = juce::jmin(1.0f, extraT + beatStep / beats);
        if (extraT >= 1.0f) extraMove = MoveType::None;
    }
    if (wasBusy && !isMoveBusy()) moveCooldown = 1.2f;

    if (!isMoveBusy() && queuedMove != MoveType::None)
    {
        auto next = queuedMove;
        queuedMove = MoveType::None;
        triggerMove(next);
    }

    const int effectiveBar = bar >= 0 ? bar : (int) (freeBeat / 4.0);
    if (effectiveBar != lastDanceBar)
    {
        lastDanceBar = effectiveBar;
        if (!isMoveBusy() && moveCooldown <= 0.0f && effectiveBar % 2 == 0 && (energy > 0.16f || feed > 0.5f))
        {
            const MoveType mellow[] = { MoveType::HeadBop, MoveType::Shuffle, MoveType::Shimmy, MoveType::FigureEight };
            const MoveType lively[] = { MoveType::Worm, MoveType::Twerk, MoveType::Roll, MoveType::Shuffle, MoveType::Spin, MoveType::FigureEight, MoveType::Shimmy, MoveType::HeadBop };
            const bool big = !gentleMotion && (hype > 0.72f || feed > 0.5f);
            triggerMove(big ? lively[danceIndex % 8] : mellow[danceIndex % 4]);
            ++danceIndex;
        }
    }
    if (feed > 0.5f && !isMoveBusy() && moveCooldown <= 0.0f)
    {
        triggerMove(danceIndex++ % 2 == 0 ? MoveType::Twerk : MoveType::Worm);
    }

    if (!isMoveBusy() && moveCooldown <= 0.0f && mouseActive && mouseFlipCooldown <= 0.0f
        && ((facingRight && mousePos.x < fishCx - 45.0f) || (!facingRight && mousePos.x > fishCx + 45.0f)))
        triggerMove(MoveType::Flip);

    float desiredGaze = 0.0f;
    if (mouseActive && !sleepy && !isMoveBusy() && startleT <= 0.0f)
    {
        float dx = mousePos.x - fishCx, dy = mousePos.y - fishCy;
        if (std::hypot(dx, dy) > 25.0f && std::hypot(dx, dy) < 340.0f)
            desiredGaze = juce::jlimit(-0.25f, 0.25f, std::atan2(dy, facingRight ? dx : -dx) * 0.22f);
    }
    gazeAngle += (desiredGaze - gazeAngle) * (1.0f - std::exp(-dt / 0.14f));
    shadesT = juce::jlimit(0.0f, 1.0f, shadesT + dt * (glassesOn ? 1.8f : -2.2f));
    repaint();
}

void KoiFish::triggerMove(MoveType move)
{
    if (move == MoveType::None) return;
    if (isMoveBusy())
    {
        const bool same = (move == MoveType::Worm && wormActive) || (move == MoveType::Roll && rollActive)
            || (move == MoveType::Spin && partyActive) || (move == MoveType::Flip && flipActive) || move == extraMove;
        if (!same) queuedMove = move; // Latest request wins, after a clean landing.
        return;
    }
    if (beatPhase >= 0.0f && danceBeat > 0.06f && danceBeat < 0.98f && beatIdleTime < 0.2f)
    {
        queuedMove = move;
        return; // Pick up the next downbeat instead of entering mid-step.
    }
    queuedMove = MoveType::None;
    mouseFlipCooldown = 2.5f;
    switch (move)
    {
        case MoveType::Worm: wormActive = true; wormT = 0.0f; break;
        case MoveType::Roll: rollActive = true; rollT = 0.0f; break;
        case MoveType::Spin: partyActive = true; partyT = 0.0f; break;
        case MoveType::Flip: flipActive = true; flipT = 0.0f; flipMid = false; break;
        default: extraMove = move; extraT = 0.0f; break;
    }
}

void KoiFish::setMouseTarget(juce::Point<float> pos, bool inWindow)
{
    mousePos = pos;
    mouseActive = inWindow;
}

void KoiFish::triggerStartle(juce::Point<float> tapPos)
{
    startleT = 1.0f;
    tailBurst = 1.0f;
    auto b = getLocalBounds().toFloat();
    juce::Point<float> fishCentre { b.getWidth() * 0.5f, b.getHeight() * 0.52f };
    juce::Point<float> diff = fishCentre - tapPos;
    float len = std::hypot(diff.x, diff.y);
    if (len > 1e-3f)
        startleVec = { diff.x / len, diff.y / len };
    else
        startleVec = { (facingRight ? -1.0f : 1.0f), -0.5f };
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
    const float phase = (float) swimPhase;
    const float breathe = 1.0f + 0.05f * std::sin((float) time * 1.1f) * (1.0f - energy);

    float wormEnv = wormActive ? std::pow(std::sin(kPi * juce::jlimit(0.0f, 1.0f, wormT)), 2.0f) : 0.0f;
    float wormPhase = wormT * 8.0f * kPi;

    for (int i = 0; i < N; ++i)
    {
        float u = (float) i / (float) (N - 1);
        float ampU = bendAmp * (0.12f + 0.88f * (1.0f - u));
        float wy = 15.0f + ampU * std::sin(phase - u * 4.4f);
        if (wormActive)
            wy += std::sin(wormPhase - u * 5.2f) * wormEnv * 6.5f;
        const float envelope = std::pow(std::sin(kPi * extraT), 2.0f) * (gentleMotion ? 0.35f : 1.0f);
        if (extraMove == MoveType::Twerk)
            wy += (-2.0f + 5.5f * std::cos(danceBeat * 2.0f * kPi)) * std::pow(1.0f - u, 1.6f) * envelope;
        if (extraMove == MoveType::Shimmy)
            wy += std::sin(danceBeat * 4.0f * kPi - u * 4.0f) * 2.5f * (1.0f - u) * envelope;

        const float hipThrust = extraMove == MoveType::Twerk
            ? 1.8f * std::sin(danceBeat * 2.0f * kPi) * std::pow(1.0f - u, 2.0f) * envelope : 0.0f;
        sp[(size_t) i] = { 7.0f + u * 22.0f + hipThrust,
                           wy,
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
        // Keep the eye underneath the glasses throughout their flight.
        {
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

                int glintDx = 0;
                int glintDy = -1;
                if (mouseActive)
                {
                    auto b = getLocalBounds().toFloat();
                    float cy = b.getHeight() * 0.52f;
                    if (mousePos.y > cy + 25.0f) glintDy = 1;
                    else if (mousePos.y < cy - 25.0f) glintDy = -1;
                    else glintDy = 0;

                    float cx = b.getWidth() * 0.5f;
                    if ((mousePos.x > cx) == facingRight) glintDx = 1;
                    else glintDx = 0;
                }
                set(ex + glintDx, ey + glintDy, 'g');
            }
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

        eyeGX = ex;
        eyeGY = ey;
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
    const int pixel = juce::jmax(1, (int) (juce::jmin(w, h) / (float) (GRID_W + 10)));

    std::vector<char> grid;
    buildGrid(grid);

    float flipScale = flipActive ? std::cos(kPi * flipT) : 1.0f;
    float aScale = std::abs(flipScale);
    const float beatWeight = juce::jlimit(0.0f, 1.0f, energy * 1.5f + pulse * 0.35f);
    const float lift = 0.5f - 0.5f * std::cos(2.0f * kPi * danceBeat);
    const float impact = std::pow(1.0f - lift, 3.0f) * beatWeight;
    float squashX = aScale * (1.0f + 0.18f * impact - 0.05f * lift * beatWeight);
    float squashY = (1.0f + 0.22f * (1.0f - aScale)) * (1.0f - 0.19f * impact + 0.10f * lift * beatWeight);

    float hopScale = beatPhase >= 0.0f ? 0.25f : 1.0f;
    float hop = -(7.0f + 12.0f * energy) * pulse * pulse * hopScale
              - (flipActive ? 10.0f * std::sin(kPi * flipT) : 0.0f);

    float bob = 0.0f;
    if (beatPhase >= 0.0f || energy > 0.12f)
        bob = (0.25f * impact - lift * beatWeight) * (8.0f + 44.0f * energy) * juce::jmin(1.0f, h / 388.0f);
    else
        bob = std::sin(driftPhase * 1.5) * (3.3f + 6.7f * energy) * (sleepy ? 0.5f : 1.0f);

    float sway = std::sin(driftPhase) * (1.2f + 2.4f * energy) * (sleepy ? 0.5f : 1.0f);
    if (extraMove == MoveType::FigureEight)
    {
        const float settling = std::pow(std::sin(kPi * extraT), 2.0f);
        bob *= 1.0f - 0.85f * settling;
        sway *= 1.0f - settling;
    }
    if (extraMove == MoveType::Twerk) bob *= 0.25f;
    float cx = w * 0.5f + sway;
    float cy = h * 0.52f + bob + hop;
    fishCx = cx;
    fishCy = cy;

    float spinA = gazeAngle;
    if (partyActive)
    {
        float p = juce::jlimit(0.0f, 1.0f, partyT);
        float eased = p < 0.5f ? 4.0f * p * p * p : 1.0f - std::pow(-2.0f * p + 2.0f, 3.0f) * 0.5f;
        spinA += eased * 2.0f * kPi;
    }

    // Dance Move 1: The Worm (Breakdance body ripple)
    if (wormActive)
    {
        float p = juce::jlimit(0.0f, 1.0f, wormT);
        float wormEnv = std::pow(std::sin(kPi * p), 2.0f);
        cy += std::sin(p * 2.0f * kPi) * 14.0f * wormEnv;
        spinA += std::sin(p * 2.0f * kPi) * 0.22f * (facingRight ? 1.0f : -1.0f) * wormEnv;
    }

    // Dance Move 2: The Barrel Roll (3D corkscrew loop)
    if (rollActive)
    {
        float p = juce::jlimit(0.0f, 1.0f, rollT);
        float eased = p < 0.5f ? 4.0f * p * p * p : 1.0f - std::pow(-2.0f * p + 2.0f, 3.0f) * 0.5f;
        float theta = eased * 2.0f * kPi;
        float loopR = juce::jmin(28.0f, h * 0.065f);
        float dir = facingRight ? 1.0f : -1.0f;
        cx += std::sin(theta) * loopR * dir;
        cy += -(1.0f - std::cos(theta)) * loopR;
        spinA += dir * theta;
        squashY *= (0.88f + 0.12f * std::cos(theta));
        squashX *= (1.0f + 0.08f * std::sin(theta));
    }

    const float env = std::pow(std::sin(kPi * extraT), 2.0f);
    const float beat = danceBeat * 2.0f * kPi;
    const float direction = facingRight ? 1.0f : -1.0f;
    const float travel = juce::jmin(1.0f, h / 350.0f);
    switch (extraMove)
    {
        case MoveType::Shuffle:
            cx += std::sin(beat) * 26.0f * env * travel;
            spinA += std::sin(beat) * 0.16f * env;
            break;
        case MoveType::HeadBop:
            spinA += std::cos(beat) * 0.28f * direction * env;
            cy += std::cos(beat) * 8.0f * env * travel;
            break;
        case MoveType::Shimmy:
            spinA += std::sin(beat * 2.0f) * 0.07f * env;
            break;
        case MoveType::FigureEight:
        {
            const auto offset = figureEightPath(extraT, w, h);
            cx += offset.x;
            cy += offset.y;
            spinA += std::sin(extraT * 4.0f * kPi) * 0.30f * env;
            squashX *= 1.0f - 0.20f * env;
            squashY *= 1.0f - 0.20f * env;
            // A short bubble wake makes both lobes legible as he crosses them.
            if (!gentleMotion)
            {
                juce::Graphics::ScopedSaveState wakeState(g);
                for (int i = 1; i <= 32; ++i)
                {
                    const float previous = extraT - i * 0.012f;
                    if (previous < 0.0f) break;
                    const auto point = figureEightPath(previous, w, h);
                    const float alpha = 0.24f * env * (1.0f - i / 33.0f);
                    g.setColour(juce::Colour(0xffa6dde6).withAlpha(alpha));
                    g.fillEllipse(w * 0.5f + point.x - 2, h * 0.52f + point.y - 2, 4, 4);
                }
            }
            break;
        }
        case MoveType::Twerk:
        {
            const float tilt = direction * (0.48f + 0.09f * std::cos(beat)) * env;
            const float headReach = 8.0f * pixel * squashX;
            spinA += tilt;
            cx += direction * headReach * (1.0f - std::cos(tilt));
            cy -= direction * headReach * std::sin(tilt);
            break;
        }
        default: break;
    }

    // Startle & Cursor tracking (disabled while busy dancing)
    if (startleT > 0.0f)
    {
        float sEase = startleT * startleT;
        cx += startleVec.x * 28.0f * sEase;
        cy += startleVec.y * 22.0f * sEase;
        spinA += (facingRight ? -0.28f : 0.28f) * sEase;
    }
    if (gentleMotion)
    {
        cx = w * 0.5f + (cx - w * 0.5f) * 0.25f;
        cy = h * 0.52f + (cy - h * 0.52f) * 0.25f;
        // Keep complete rotations complete; suppress acrobatics in gentle mode.
        spinA = gazeAngle * 0.25f;
        squashX = aScale;
        squashY = 1.0f;
    }
    fishCx = cx;
    fishCy = cy;
    float ox = std::floor(w * 0.5f - GRID_W * 0.5f * pixel);
    float oy = std::floor(h * 0.52f - GRID_H * 0.5f * pixel);
    const float originX = ox + GRID_W * 0.5f * pixel;
    const float originY = oy + GRID_H * 0.5f * pixel;
    // Rasterise a connected pixel sprite once, then transform it as a whole.
    // Scaling pixel positions individually leaves holes between fixed-size cells.
    const auto fishTransform = juce::AffineTransform::translation(-originX, -originY)
        .scaled(squashX, squashY).rotated(spinA).translated(cx, cy);

    if (spriteImg.isNull() || spriteImg.getWidth() != (int) w || spriteImg.getHeight() != (int) h)
        spriteImg = juce::Image(juce::Image::ARGB, (int) w, (int) h, true, juce::SoftwareImageType());
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
                float px = ox + sx * (float) pixel;
                float py = oy + gy * (float) pixel;
                ig.setColour(colourFor(c));
                ig.fillRect(px, py, (float) pixel, (float) pixel);
            }
        }
    }

    {
        juce::Graphics::ScopedSaveState state(g);
        g.addTransform(fishTransform);
        g.setImageResamplingQuality(juce::Graphics::mediumResamplingQuality);
        g.drawImageAt(spriteImg, 0, 0);
    }

    const int msx = facingRight ? mouthGX : GRID_W - 1 - mouthGX;
    mouthPos = juce::Point<float>(ox + (msx + 0.5f) * pixel, oy + (mouthGY + 0.5f) * pixel).transformedBy(fishTransform);

    if (shadesT > 0.005f)
    {
        const float flyEase = shadesT * shadesT * (3.0f - 2.0f * shadesT);
        const float flyOffset = (1.0f - flyEase) * -(h + 80.0f);
        const int esx = facingRight ? eyeGX : GRID_W - 1 - eyeGX;
        // Anchor the visible lens to the same eye cell and affine transform.
        // The raster face itself is axis-aligned, so an extra tangent rotation
        // would make the glasses slide across it during spine bends.
        const float eyePx = ox + (esx + 0.5f) * pixel;
        const float eyePy = oy + (eyeGY + 0.5f) * pixel;
        const float cSizeX = pixel * 0.49f;
        const float cSizeY = cSizeX;
        const float lensColCenter = facingRight ? 10.0f : 18.0f;
        const float gx0 = eyePx - lensColCenter * cSizeX;
        const float gy0 = eyePy - 1.5f * cSizeY;

        const juce::Colour colShadesBlack { 0xFF0D0A08 };
        const juce::Colour colShadesWhite { 0xFFFFFFFF };

        juce::Graphics::ScopedSaveState shadesState(g);
        g.addTransform(fishTransform.followedBy(juce::AffineTransform::translation(0.0f, flyOffset)));

        for (int r = 0; r < 4; ++r)
        {
            for (int c = 0; c < 29; ++c)
            {
                int srcC = facingRight ? (28 - c) : c;
                char ch = kDealWithItArt[r][srcC];
                if (ch == '.') continue;

                float bx = gx0 + (float) c * cSizeX;
                float by = gy0 + (float) r * cSizeY;

                g.setColour(ch == 'W' ? colShadesWhite : colShadesBlack);
                g.fillRect(bx, by, cSizeX, cSizeY);
            }
        }

        // Meme corner sparkle glint when fully landed
        if (shadesT >= 0.92f)
        {
            float spRate = (float) std::fmod(time * 2.8, 1.0);
            float spAlpha = std::sin(spRate * kPi);
            if (spAlpha > 0.05f)
            {
                float spCol = facingRight ? 11.0f : 17.0f;
                float spX = gx0 + spCol * cSizeX;
                float spY = gy0 + 1.0f * cSizeY;
                float spW = cSizeX * 0.85f;
                float spH = cSizeY * 0.85f;

                g.setColour(juce::Colours::white.withAlpha(spAlpha * 0.95f));
                g.fillRect(spX - spW * 1.5f, spY - spH * 0.35f, spW * 3.0f, spH * 0.7f);
                g.fillRect(spX - spW * 0.35f, spY - spH * 1.5f, spW * 0.7f, spH * 3.0f);
            }
        }
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
