#include "KoiFish.h"
#include <cmath>
#include <utility>

namespace
{
    constexpr float kPi = 3.14159265f;

    // Shared eased timing keeps deformation, contact and rotation in one phrase.
    float danceEase(float start, float end, float progress)
    {
        const float t = juce::jlimit(0.0f, 1.0f, (progress - start) / (end - start));
        return t * t * (3.0f - 2.0f * t);
    }

    float smoother(float t)
    {
        t = juce::jlimit(0.0f, 1.0f, t);
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    // Beat-locked accent: 1 on the beat, easing to 0 by the off-beat. Smooth everywhere.
    float onBeat(float beatPhase, float sharpness = 3.0f)
    {
        return std::pow(0.5f + 0.5f * std::cos(2.0f * kPi * beatPhase), sharpness);
    }

    // Keeps a scale's sign but never lets the sprite collapse to an invisible line.
    float visibleScale(float s, float minimum = 0.10f)
    {
        return s >= 0.0f ? juce::jmax(s, minimum) : juce::jmin(s, -minimum);
    }

    float breakdancePlant(float progress)
    {
        return danceEase(0.0f, 0.22f, progress) * (1.0f - danceEase(0.78f, 1.0f, progress));
    }

    float breakdanceSpin(float progress)
    {
        return danceEase(0.22f, 0.32f, progress) * (1.0f - danceEase(0.58f, 0.70f, progress));
    }

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
    return activeMove != MoveType::None;
}

float KoiFish::moveBeats(MoveType move)
{
    switch (move)
    {
        case MoveType::Flip: return 1.0f;
        case MoveType::FigureEight:
        case MoveType::Twerk: return 8.0f;
        default: return 4.0f;
    }
}

juce::Point<float> KoiFish::figureEightPath(float progress, float width, float height)
{
    // Four eased arcs: one landmark every two beats, without shrinking either
    // lobe. Multiplying the coordinates by an envelope distorts the eight.
    const float arcs = juce::jlimit(0.0f, 1.0f, progress) * 4.0f;
    const float whole = std::floor(arcs);
    const float fraction = arcs - whole;
    // Half-eased: he still hits a landmark every two beats, but keeps gliding
    // through it instead of stopping dead.
    const float eased = 0.5f * fraction + 0.5f * fraction * fraction * (3.0f - 2.0f * fraction);
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
    const bool chasing = foodTarget.has_value();
    calmTime = energy < 0.06f && pulse < 0.12f && feed < 0.5f && !chasing ? calmTime + dt : 0.0f;
    sleepy = calmTime > 6.0f;
    moveCooldown = juce::jmax(0.0f, moveCooldown - dt);
    breakdanceCooldown = juce::jmax(0.0f, breakdanceCooldown - dt);
    mouseFlipCooldown = juce::jmax(0.0f, mouseFlipCooldown - dt);
    startleT = juce::jmax(0.0f, startleT - dt * 2.2f);

    const bool wasBusy = isMoveBusy();
    if (activeMove != MoveType::None)
    {
        moveT = juce::jmin(1.0f, moveT + beatStep / moveBeats(activeMove));
        if (activeMove == MoveType::Flip && !flipMid && moveT >= 0.5f) { facingRight = !facingRight; flipMid = true; }
        if (moveT >= 1.0f) activeMove = MoveType::None;
    }
    if (wasBusy && !isMoveBusy()) moveCooldown = 1.2f;

    if (!isMoveBusy() && queuedMove != MoveType::None)
    {
        auto next = queuedMove;
        queuedMove = MoveType::None;
        triggerMove(next);
    }

    // Dinner comes first: automatic choreography waits until the food is gone.
    const int effectiveBar = bar >= 0 ? bar : (int) (freeBeat / 4.0);
    if (effectiveBar != lastDanceBar)
    {
        lastDanceBar = effectiveBar;
        if (!chasing && !isMoveBusy() && moveCooldown <= 0.0f && effectiveBar % 2 == 0 && (energy > 0.16f || feed > 0.5f))
        {
            const MoveType mellow[] = { MoveType::HeadBop, MoveType::Shuffle, MoveType::Shimmy, MoveType::FigureEight };
            const MoveType lively[] = { MoveType::Worm, MoveType::Twerk, MoveType::Roll, MoveType::Shuffle, MoveType::Spin, MoveType::FigureEight, MoveType::Shimmy, MoveType::HeadBop };
            const bool big = !gentleMotion && (hype > 0.72f || feed > 0.5f);
            triggerMove(big ? lively[danceIndex % 8] : mellow[danceIndex % 4]);
            ++danceIndex;
        }
    }
    if (!chasing && feed > 0.5f && !isMoveBusy() && moveCooldown <= 0.0f)
    {
        triggerMove(danceIndex++ % 2 == 0 ? MoveType::Twerk : MoveType::Worm);
    }

    if (chasing) {}
    else if (!gentleMotion && !isMoveBusy() && breakdanceCooldown <= 0.0f && pulse > 0.94f && energy > 0.65f && intensity == 2)
    {
        triggerMove(MoveType::Breakdance);
    }
    else if (!isMoveBusy() && moveCooldown <= 0.0f && pulse > 0.88f && energy > 0.40f && intensity >= 1)
    {
        triggerMove(MoveType::Flip);
    }

    if (!chasing && !isMoveBusy() && moveCooldown <= 0.0f && mouseActive && mouseFlipCooldown <= 0.0f
        && ((facingRight && mousePos.x < fishCx - 45.0f) || (!facingRight && mousePos.x > fishCx + 45.0f)))
        triggerMove(MoveType::Flip);

    // Chase: spring the roaming offset so the mouth, not the body centre, meets the pellet.
    {
        const auto bounds = getLocalBounds().toFloat();
        const juce::Point<float> rest { bounds.getWidth() * 0.5f, bounds.getHeight() * 0.52f };
        juce::Point<float> desired;
        if (chasing)
        {
            const auto mouthOffset = mouthPos - juce::Point<float>(fishCx, fishCy);
            desired = *foodTarget - rest - mouthOffset;
            // Keep the whole body in the tank, not just the mouth.
            const float pixel = (float) juce::jmax(1, (int) (juce::jmin(bounds.getWidth(), bounds.getHeight()) / (float) (GRID_W + 10)));
            const float halfLen = GRID_W * pixel * 0.52f, halfTall = GRID_H * pixel * 0.20f;
            desired.x = juce::jlimit(halfLen - rest.x, bounds.getWidth() - halfLen - rest.x, desired.x);
            desired.y = juce::jlimit(halfTall - rest.y, bounds.getHeight() - halfTall * 0.5f - rest.y, desired.y);
            // Food behind the head: turn around first instead of swimming backwards.
            const bool behind = (facingRight && foodTarget->x < mouthPos.x - 12.0f) || (!facingRight && foodTarget->x > mouthPos.x + 12.0f);
            if (behind)
            {
                desired.x = roam.x;
                if (!isMoveBusy()) triggerMove(MoveType::Flip);
            }
        }
        const float stiffness = chasing ? 26.0f : 5.0f;
        const auto accel = (desired - roam) * stiffness - roamVel * (2.0f * std::sqrt(stiffness));
        roamVel += accel * dt;
        const float maxSpeed = gentleMotion ? 140.0f : 280.0f;
        const float speed = roamVel.getDistanceFromOrigin();
        if (speed > maxSpeed) roamVel *= maxSpeed / speed;
        roam += roamVel * dt;
        if (chasing) tailBurst = juce::jmax(tailBurst, juce::jmin(0.8f, speed / 300.0f));
        chaseAmt += ((chasing ? 1.0f : 0.0f) - chaseAmt) * (1.0f - std::exp(-dt / 0.25f));
    }
    chompT = juce::jmax(0.0f, chompT - dt / 0.35f);
    fullness = juce::jmax(0.0f, fullness - dt / 40.0f);
    for (auto& heart : hearts)
        if (heart.age < 1.0f) { heart.age += dt / 1.4f; heart.y -= dt * 34.0f; }

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
        if (move != activeMove) queuedMove = move; // Latest request wins, after a clean landing.
        return;
    }
    if (beatPhase >= 0.0f && danceBeat > 0.06f && danceBeat < 0.98f && beatIdleTime < 0.2f)
    {
        queuedMove = move;
        return; // Pick up the next downbeat instead of entering mid-step.
    }
    queuedMove = MoveType::None;
    mouseFlipCooldown = 2.5f;
    activeMove = move;
    moveT = 0.0f;
    flipMid = false;
    if (move == MoveType::Breakdance)
    {
        breakdanceJustTriggered = true;
        breakdanceCooldown = 10.0f;
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

void KoiFish::chomp()
{
    chompT = 1.0f;
    tailBurst = juce::jmax(tailBurst, 0.6f);
    fullness = juce::jmin(1.0f, fullness + 0.07f);
    auto& heart = hearts[(size_t) (nextHeart++ % (int) hearts.size())];
    heart = { mouthPos.x + (facingRight ? -6.0f : 6.0f), mouthPos.y - 12.0f, 0.0f };
}

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

    float wormEnv = doing(MoveType::Worm) ? std::pow(std::sin(kPi * juce::jlimit(0.0f, 1.0f, moveT)), 2.0f) : 0.0f;
    float wormPhase = moveT * 8.0f * kPi;

    for (int i = 0; i < N; ++i)
    {
        float u = (float) i / (float) (N - 1);
        float ampU = bendAmp * (0.12f + 0.88f * (1.0f - u));
        float wy = 15.0f + ampU * std::sin(phase - u * 4.4f);
        if (doing(MoveType::Worm))
            wy += std::sin(wormPhase - u * 5.2f) * wormEnv * 6.5f;
        // Every move bends the pixel body itself, like the worm: the spine
        // carries the gesture and the rigid transform in paint() only frames it.
        const float gentleK = gentleMotion ? 0.35f : 1.0f;
        const float envelope = std::pow(std::sin(kPi * moveT), 2.0f) * gentleK;
        const float curlShape = 4.0f * (u - 0.5f) * (u - 0.5f) - 0.33f; // ends bend, middle holds
        switch (activeMove)
        {
            case MoveType::Twerk:
                wy += (-2.0f + 5.5f * (2.0f * onBeat(danceBeat, 1.5f) - 1.0f)) * std::pow(1.0f - u, 1.6f) * envelope;
                break;
            case MoveType::Shimmy:
                // See-saw shake twice a beat: head and tail opposite, middle steady.
                wy += envelope * (5.6f * std::sin(4.0f * kPi * danceBeat) * std::cos(kPi * u)
                                  + 1.2f * std::sin(4.0f * kPi * danceBeat - u * 6.0f));
                break;
            case MoveType::HeadBop:
            {
                const float nod = onBeat(danceBeat) * envelope;
                wy += 7.0f * nod * std::pow(u, 2.2f) - 3.0f * nod * std::pow(1.0f - u, 2.0f);
                break;
            }
            case MoveType::Spin:
                wy += 5.5f * envelope * curlShape;
                break;
            case MoveType::Flip:
                wy += 5.0f * std::sin(kPi * moveT) * gentleK * curlShape;
                break;
            case MoveType::Roll:
                wy += 3.4f * envelope * std::sin(moveT * 8.0f * kPi - u * 5.2f);
                break;
            case MoveType::Shuffle:
                wy += 2.0f * envelope * std::sin(moveT * 8.0f * kPi - u * 4.5f);
                break;
            case MoveType::FigureEight:
                wy += 2.4f * envelope * std::sin(moveT * 16.0f * kPi - u * 5.0f);
                break;
            default: break;
        }

        float bdThrust = 0.0f;
        if (doing(MoveType::Breakdance))
        {
            const float bt = juce::jlimit(0.0f, 1.0f, moveT);
            const float motion = gentleMotion ? 0.20f : 1.0f;
            const float spin = breakdanceSpin(bt) * motion;
            const float freeze = danceEase(0.62f, 0.72f, bt) * breakdancePlant(bt) * motion;
            const float whipAngle = danceEase(0.22f, 0.70f, bt) * 4.0f * kPi - u * 3.0f;
            // A modest trailing tail, with a stable head, preserves the silhouette.
            wy += std::sin(whipAngle) * (1.0f - u) * 2.8f * spin;
            bdThrust = std::cos(whipAngle) * (1.0f - u) * 0.8f * spin;
            wy += (std::sin(u * kPi) * 2.5f - (1.0f - u) * 1.5f) * freeze;
        }

        const float hipThrust = activeMove == MoveType::Twerk
            ? 1.8f * std::sin(danceBeat * 2.0f * kPi) * std::pow(1.0f - u, 2.0f) * envelope : 0.0f;
        sp[(size_t) i] = { 7.0f + u * 22.0f + hipThrust + bdThrust,
                           wy,
                           (1.4f + 4.2f * std::pow(u, 0.6f)
                            + 1.3f * fullness * std::exp(-std::pow((u - 0.56f) / 0.17f, 2.0f))) * breathe,
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

        // Capped so the upper lobe never swings past vertical at full energy.
        float wag = juce::jlimit(-0.6f, 0.6f, (0.35f + 0.45f * energy + 0.9f * tailBurst) * std::sin(phase * 1.12f + 0.8f));

        auto lobe = [&](float vy)
        {
            float dx = -1.0f, dy = vy;
            float len = std::sqrt(dx * dx + dy * dy);
            dx /= len; dy /= len;
            float c = std::cos(wag), s = std::sin(wag);
            float rdx = dx * c - dy * s, rdy = dx * s + dy * c;
            float px = -rdy, py = rdx;
            const float L = 9.0f;
            // Scan-fill the lobe in grid space. Stamping rotated strokes left
            // holes at steep angles, which the outline pass turned into a lattice.
            const int x0 = (int) std::floor(j.x - L - 3.0f), x1 = (int) std::ceil(j.x + 3.0f);
            const int y0 = (int) std::floor(j.y - L - 3.0f), y1 = (int) std::ceil(j.y + L + 3.0f);
            for (int gy = y0; gy <= y1; ++gy)
            {
                for (int gx = x0; gx <= x1; ++gx)
                {
                    const float ox = (float) gx - j.x, oy = (float) gy - j.y;
                    const float along = ox * rdx + oy * rdy;
                    const float across = ox * px + oy * py;
                    if (along < 0.5f || along > L + 0.5f) continue;
                    const float halfW = 0.6f + 2.4f * std::sin(kPi * 0.92f * juce::jmin(along, L) / L);
                    if (std::abs(across) > halfW + 0.35f) continue;
                    const float side = py * across;
                    char cc = side < -0.2f ? '5' : side > 0.2f ? '6' : 'f';
                    if (along > L - 0.5f && std::abs(across) > halfW - 0.8f) cc = '6';
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
        if (chompT > 0.45f)
        {
            // Gulp: a round open mouth for the first beat of a bite.
            set(mx - 1, my - 1, 'm');
            set(mx, my - 1, 'm');
            set(mx - 1, my + 1, 'm');
            set(mx, my + 1, 'm');
        }
        mouthGX = mx - 1;
        mouthGY = my;

        auto pb = spineAt(0.93f);
        int bxx = (int) std::round(pb.x);
        int byy = (int) std::round(pb.y + pb.r * 0.55f);
        set(bxx, byy, 'b');
        set(bxx + 1, byy, 'b');
        if (fullness > 0.45f) { set(bxx - 1, byy, 'b'); set(bxx, byy + 1, 'b'); }

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

    const float flipEase = moveT * moveT * (3.0f - 2.0f * moveT);
    float flipScale = doing(MoveType::Flip) ? std::cos(kPi * flipEase) : 1.0f;
    float aScale = juce::jmax(0.10f, std::abs(flipScale)); // never an invisible sliver mid-turn
    const float beatWeight = juce::jlimit(0.0f, 1.0f, energy * 1.5f + pulse * 0.35f);
    const float lift = 0.5f - 0.5f * std::cos(2.0f * kPi * danceBeat);
    const float impact = std::pow(1.0f - lift, 3.0f) * beatWeight;
    float squashX = aScale * (1.0f + 0.18f * impact - 0.05f * lift * beatWeight);
    float squashY = (1.0f + 0.22f * (1.0f - aScale)) * (1.0f - 0.19f * impact + 0.10f * lift * beatWeight);

    float hopScale = beatPhase >= 0.0f ? 0.25f : 1.0f;
    float hop = -(7.0f + 12.0f * energy) * pulse * pulse * hopScale
              - (doing(MoveType::Flip) ? 14.0f * std::sin(kPi * moveT) : 0.0f);

    float bob = 0.0f;
    if (beatPhase >= 0.0f || energy > 0.12f)
        bob = (0.25f * impact - lift * beatWeight) * (8.0f + 44.0f * energy) * juce::jmin(1.0f, h / 388.0f);
    else
        bob = std::sin(driftPhase * 1.5) * (3.3f + 6.7f * energy) * (sleepy ? 0.5f : 1.0f);

    float sway = std::sin(driftPhase) * (1.2f + 2.4f * energy) * (sleepy ? 0.5f : 1.0f);
    if (activeMove == MoveType::FigureEight)
    {
        const float settling = std::pow(std::sin(kPi * moveT), 2.0f);
        bob *= 1.0f - 0.85f * settling;
        sway *= 1.0f - settling;
    }
    if (activeMove == MoveType::Twerk) bob *= 0.25f;
    // Chasing food is purposeful swimming, not dancing: damp the idle drift.
    bob *= 1.0f - 0.8f * chaseAmt;
    sway *= 1.0f - 0.8f * chaseAmt;
    float cx = w * 0.5f + sway;
    float cy = h * 0.52f + bob + hop;
    fishCx = cx;
    fishCy = cy;

    float spinA = gazeAngle;
    const float travel = juce::jmin(1.0f, h / 350.0f);
    const float direction = facingRight ? 1.0f : -1.0f;
    const float env = std::pow(std::sin(kPi * moveT), 2.0f);

    // Spin: curl into a C (spine), tuck, pirouette once, uncurl.
    if (doing(MoveType::Spin))
    {
        spinA += direction * 2.0f * kPi * smoother((moveT - 0.18f) / 0.64f);
        squashX *= 1.0f - 0.10f * env;
        squashY *= 1.0f - 0.10f * env;
        cy -= 14.0f * env * travel;
    }

    // Dance Move 1: The Worm (Breakdance body ripple)
    if (doing(MoveType::Worm))
    {
        float p = juce::jlimit(0.0f, 1.0f, moveT);
        float wormEnv = std::pow(std::sin(kPi * p), 2.0f);
        cy += std::sin(p * 2.0f * kPi) * 14.0f * wormEnv;
        spinA += std::sin(p * 2.0f * kPi) * 0.22f * (facingRight ? 1.0f : -1.0f) * wormEnv;
    }

    // Dance Move 2: The Barrel Roll (3D corkscrew loop)
    // Barrel roll: a real roll about the long axis. The sprite's height runs
    // through zero to belly-up and back while he corkscrews up and over.
    if (doing(MoveType::Roll))
    {
        // Quick half-roll to belly-up, two beats of upside-down swimming, roll home.
        const float theta = kPi * (smoother((moveT - 0.10f) / 0.20f) + smoother((moveT - 0.70f) / 0.20f));
        squashY *= visibleScale(std::cos(theta), 0.08f);
        squashX *= 1.0f + 0.07f * std::abs(std::sin(theta));
        cx += direction * std::sin(theta) * 20.0f * travel;
        cy += (-30.0f * env + 10.0f * std::sin(theta)) * travel;
        spinA += direction * 0.22f * std::sin(theta);
    }

    // Dance Move: Breakdance (Headspin to Freeze combo)
    if (doing(MoveType::Breakdance))
    {
        const float p = juce::jlimit(0.0f, 1.0f, moveT);
        const float dir = facingRight ? 1.0f : -1.0f;
        const float plant = breakdancePlant(p);
        const float spin = breakdanceSpin(p);
        const float angle = danceEase(0.22f, 0.70f, p) * 4.0f * kPi;
        const float freeze = danceEase(0.62f, 0.72f, p);
        spinA = spinA * (1.0f - plant)
            + dir * (0.5f * kPi - 0.08f * kPi * freeze) * plant
            + dir * std::sin(angle) * 0.10f * spin;
        // Once upright, local X is the fish's length. Compress its width (Y)
        // gently to suggest a headspin without repeatedly telescoping the body.
        squashX += (1.0f - squashX) * plant;
        // Upright, local Y is his width: cycling it through zero reads as a
        // spin about the vertical axis. Four turns during the spin window.
        const float headspin = visibleScale(1.0f + (std::cos(2.0f * angle) - 1.0f) * spin, 0.12f);
        squashY += (headspin - squashY) * plant;
    }

    const float beat = danceBeat * 2.0f * kPi;
    switch (activeMove)
    {
        case MoveType::Shuffle:
        {
            // Beat-stepped slides: centre, right, left, right, centre. Each step
            // glides in the first half of the beat, hops, and leans into travel.
            static constexpr float stops[5] = { 0.0f, 1.0f, -1.0f, 1.0f, 0.0f };
            const float beats = juce::jlimit(0.0f, 3.999f, moveT * 4.0f);
            const int k = (int) beats;
            const float f = juce::jmin(1.0f, (beats - (float) k) / 0.55f);
            const float from = stops[k], to = stops[k + 1];
            const float glide = std::sin(kPi * f);
            cx += (from + (to - from) * smoother(f)) * 44.0f * travel;
            cy -= glide * 12.0f * travel;
            spinA += (to - from) * 0.07f * glide;
            break;
        }
        case MoveType::HeadBop:
        {
            const float nod = onBeat(danceBeat) * env;
            spinA += direction * 0.20f * nod;
            cy += 10.0f * nod * travel;
            break;
        }
        case MoveType::Shimmy:
            cx += std::sin(beat * 2.0f) * 5.0f * env * travel;
            spinA += std::sin(beat * 2.0f + 0.6f) * 0.05f * env;
            break;
        case MoveType::FigureEight:
        {
            const auto offset = figureEightPath(moveT, w, h);
            cx += offset.x;
            cy += offset.y;
            // Face the way he swims: the heading turns the sprite through its
            // edge like a flip, and he pitches nose-up or down with the path.
            const float eps = 0.004f;
            const auto velocity = figureEightPath(juce::jmin(1.0f, moveT + eps), w, h)
                                - figureEightPath(juce::jmax(0.0f, moveT - eps), w, h);
            const float speed = velocity.getDistanceFromOrigin();
            if (speed > 0.01f)
            {
                const float ux = velocity.x / speed;
                squashX *= visibleScale(juce::jlimit(-1.0f, 1.0f, ux * direction * 1.8f), 0.12f);
                spinA += juce::jlimit(-0.55f, 0.55f, std::atan2(velocity.y, std::abs(velocity.x))) * ux * env;
            }
            // A short bubble wake makes both lobes legible as he crosses them.
            if (!gentleMotion)
            {
                juce::Graphics::ScopedSaveState wakeState(g);
                for (int i = 1; i <= 32; ++i)
                {
                    const float previous = moveT - i * 0.012f;
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
            const float tilt = direction * (0.50f + 0.10f * (2.0f * onBeat(danceBeat, 1.5f) - 1.0f)) * env;
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
    cx += roam.x;
    cy += roam.y;
    fishCx = cx;
    fishCy = cy;
    float ox = std::floor(w * 0.5f - GRID_W * 0.5f * pixel);
    float oy = std::floor(h * 0.52f - GRID_H * 0.5f * pixel);
    const float originX = ox + GRID_W * 0.5f * pixel;
    const float originY = oy + GRID_H * 0.5f * pixel;
    // Rasterise a connected pixel sprite once, then transform it as a whole.
    // Scaling pixel positions individually leaves holes between fixed-size cells.
    auto fishTransform = juce::AffineTransform::translation(-originX, -originY)
        .scaled(squashX, squashY).rotated(spinA).translated(cx, cy);
    if (doing(MoveType::Breakdance) && !gentleMotion)
    {
        // Blend into a fixed head contact, then release over the last beat.
        // Use the rendered mouth so raster spine changes cannot move the pivot.
        const int mouthX = facingRight ? mouthGX : GRID_W - 1 - mouthGX;
        const auto mouth = juce::Point<float>(ox + (mouthX + 0.5f) * pixel,
            oy + (mouthGY + 0.5f) * pixel).transformedBy(fishTransform);
        const juce::Point<float> contact(w * 0.5f + (facingRight ? 1.0f : -1.0f) * 12.0f * pixel,
                                        h * 0.80f);
        const auto correction = (contact - mouth) * breakdancePlant(moveT);
        fishTransform = fishTransform.translated(correction.x, correction.y);
        cx += correction.x;
        cy += correction.y;
        fishCx = cx;
        fishCy = cy;
    }


    // The sprite buffer only covers the grid; resampling a window-sized image
    // every frame was the editor's single biggest CPU cost.
    const int spriteW = GRID_W * pixel, spriteH = GRID_H * pixel;
    if (spriteImg.isNull() || spriteImg.getWidth() != spriteW || spriteImg.getHeight() != spriteH)
        spriteImg = juce::Image(juce::Image::ARGB, spriteW, spriteH, true, juce::SoftwareImageType());
    spriteImg.clear(spriteImg.getBounds());
    spriteOrigin = { ox, oy };

    {
        juce::Graphics ig(spriteImg);
        for (int gy = 0; gy < GRID_H; ++gy)
        {
            for (int gx = 0; gx < GRID_W; ++gx)
            {
                char c = grid[(size_t) gy * GRID_W + gx];
                if (c == '.') continue;
                int sx = facingRight ? gx : (GRID_W - 1 - gx);
                ig.setColour(colourFor(c));
                ig.fillRect(sx * pixel, gy * pixel, pixel, pixel);
            }
        }
    }

    {
        juce::Graphics::ScopedSaveState state(g);
        g.setImageResamplingQuality(juce::Graphics::mediumResamplingQuality);
        g.drawImageTransformed(spriteImg, juce::AffineTransform::translation(ox, oy).followedBy(fishTransform));
    }

    // Love hearts float up after each bite.
    for (const auto& heart : hearts)
    {
        if (heart.age >= 1.0f) continue;
        static const char* heartArt[4] = { ".X.X.", "XXXXX", ".XXX.", "..X.." };
        const float cell = juce::jmax(2.0f, pixel * 0.45f);
        const float alpha = std::sin(kPi * juce::jmin(1.0f, heart.age * 1.1f));
        const float wobble = std::sin(heart.age * 9.0f) * 4.0f;
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 5; ++c)
                if (heartArt[r][c] == 'X')
                {
                    g.setColour((r == 0 || c == 0 ? juce::Colour(0xFFFF9EC4) : juce::Colour(0xFFFF4F8B)).withAlpha(alpha));
                    g.fillRect(heart.x + wobble + (c - 2.5f) * cell, heart.y + r * cell, cell, cell);
                }
    }

    const int msx = facingRight ? mouthGX : GRID_W - 1 - mouthGX;
    mouthPos = juce::Point<float>(ox + (msx + 0.5f) * pixel, oy + (mouthGY + 0.5f) * pixel).transformedBy(fishTransform);
    floorContactPos = mouthPos;

    if (doing(MoveType::Breakdance) && !gentleMotion && moveT >= 0.22f && moveT <= 0.70f)
    {
        float spinP = (moveT - 0.22f) * 25.0f;
        for (int k = 0; k < 6; ++k)
        {
            float a = spinP + (float) k * (kPi / 3.0f);
            float dist = (10.0f + 18.0f * std::fmod(spinP * 0.4f + (float) k * 0.3f, 1.0f)) * (float) pixel * 0.35f;
            float px = floorContactPos.x + std::cos(a) * dist;
            float py = floorContactPos.y + std::abs(std::sin(a)) * dist * 0.35f;
            float particleAlpha = 0.55f * breakdanceSpin(moveT) * (1.0f - std::fmod(spinP * 0.4f + (float) k * 0.3f, 1.0f));
            g.setColour(juce::Colour(0xFFD2B48C).withAlpha(particleAlpha));
            g.fillRect(px, py, (float) pixel, (float) pixel);
        }
    }
    if (doing(MoveType::Breakdance) && !gentleMotion && moveT > 0.72f && moveT <= 0.84f)
    {
        float freezeRing = (moveT - 0.72f) / 0.12f;
        float r = freezeRing * 32.0f;
        g.setColour(juce::Colour(0xFFFFE9A8).withAlpha(0.65f * std::sin(kPi * freezeRing)));
        g.drawEllipse(floorContactPos.x - r, floorContactPos.y - r * 0.35f, r * 2.0f, r * 0.7f, 2.0f);
    }

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
