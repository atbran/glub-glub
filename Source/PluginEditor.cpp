#include "PluginEditor.h"

GlubGlubEditor::GlubGlubEditor(GlubGlubProcessor& p)
    : AudioProcessorEditor(p), proc(p), drawer(p.apvts)
{
    setSize(500, 500);
    setResizable(true, true);
    setResizeLimits(320, 320, 1024, 1024);
    addAndMakeVisible(fish);
    addAndMakeVisible(bubbles);
    addAndMakeVisible(speech);
    addAndMakeVisible(drawer);
    addAndMakeVisible(hype);
    addAndMakeVisible(disco);
    addAndMakeVisible(shaker);
    disco.setInterceptsMouseClicks(false, false);
    fish.setInterceptsMouseClicks(false, false);
    drawer.onHeightChanged = [this] { resized(); };
    drawer.onMoveTriggered = [this](KoiFish::MoveType m) { fish.triggerMove(m); };
    bubbles.setInterceptsMouseClicks(false, false);
    speech.setInterceptsMouseClicks(false, false);
    startTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    startTimerHz(60);
}

GlubGlubEditor::~GlubGlubEditor() { stopTimer(); }

void GlubGlubEditor::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    float hueShift = proc.apvts.getRawParameterValue("hue")->load();
    juce::Colour top = juce::Colour::fromHSV(0.58f + hueShift, 0.55f, 0.35f, 1.0f);
    juce::Colour bottom = juce::Colour::fromHSV(0.60f + hueShift, 0.6f, 0.12f, 1.0f);
    g.setGradientFill(juce::ColourGradient(top, 0, 0, bottom, 0, b.getHeight(), false));
    g.fillAll();
    // pebbles
    g.setColour(juce::Colour(0xFF2A3B4D));
    for (int i = 0; i < 8; ++i)
    {
        float x = std::fmod(i * 173.0f, b.getWidth());
        g.fillEllipse(x, b.getHeight() - 18 - (i % 3) * 8, 26, 12);
    }
    // seaweed
    g.setColour(juce::Colour(0xFF2E8B57));
    for (int i = 0; i < 3; ++i)
    {
        float x = 30 + i * 40;
        g.fillRect(x, b.getHeight() - 90.0f, 8.0f, 70.0f);
    }

    // Sweeping Disco Lasers
    if (disco.isShowing() && disco.getAppear() > 0.05f)
    {
        float app = disco.getAppear();
        auto ballCentre = disco.getBounds().getTopLeft().toFloat() + disco.getBallCentre();
        double now = juce::Time::getMillisecondCounterHiRes() / 1000.0 - startTime;
        float beatPulse = proc.vibe.beatPulse.load();
        float baseAlpha = app * (0.35f + 0.35f * beatPulse);

        struct LaserDef {
            juce::Colour col;
            float freq;
            float phase;
            float spread;
        };
        const LaserDef lasers[4] = {
            { juce::Colour(0xFF00E5FF), 2.2f, 0.0f, 0.55f }, // Neon Cyan
            { juce::Colour(0xFFFF007F), 1.7f, 1.3f, 0.65f }, // Hot Pink
            { juce::Colour(0xFF39FF14), 2.8f, 2.7f, 0.50f }, // Electric Lime
            { juce::Colour(0xFFFFD700), 1.9f, 4.2f, 0.70f }  // Cyber Gold
        };

        for (int i = 0; i < 4; ++i)
        {
            float ang = 0.5f * 3.14159265f + lasers[i].spread * std::sin((float) now * lasers[i].freq + lasers[i].phase);
            float length = b.getHeight() * 1.5f;
            float ex = ballCentre.x + std::cos(ang) * length;
            float ey = ballCentre.y + std::sin(ang) * length;

            // Wide translucent atmospheric beam glow
            g.setColour(lasers[i].col.withAlpha(baseAlpha * 0.25f));
            g.drawLine(ballCentre.x, ballCentre.y, ex, ey, 6.0f);

            // Core intense laser beam
            g.setColour(lasers[i].col.withAlpha(baseAlpha * 0.75f));
            g.drawLine(ballCentre.x, ballCentre.y, ex, ey, 2.0f);

            // Floor / surface hit splash
            float tFloor = (b.getHeight() - 15.0f - ballCentre.y) / (std::sin(ang) * length);
            if (tFloor > 0.0f && tFloor < 1.0f)
            {
                float hx = ballCentre.x + std::cos(ang) * length * tFloor;
                float hy = b.getHeight() - 15.0f;
                g.setColour(lasers[i].col.withAlpha(baseAlpha * 0.6f));
                g.fillEllipse(hx - 8.0f, hy - 4.0f, 16.0f, 8.0f);
            }
        }
    }

    // Water Ripples from glass tapping
    for (const auto& rip : ripples)
    {
        if (rip.alpha <= 0.01f) continue;
        g.setColour(juce::Colour(0xFFBEE9E8).withAlpha(rip.alpha * 0.75f));
        g.drawEllipse(rip.x - rip.radius, rip.y - rip.radius * 0.45f, rip.radius * 2.0f, rip.radius * 0.9f, 2.0f);
        if (rip.radius > 8.0f)
        {
            g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(rip.alpha * 0.4f));
            g.drawEllipse(rip.x - (rip.radius - 6.0f), rip.y - (rip.radius - 6.0f) * 0.45f,
                          (rip.radius - 6.0f) * 2.0f, (rip.radius - 6.0f) * 0.9f, 1.0f);
        }
    }

    if (partyGlow > 0.004f)
    {
        float beatPulse = proc.vibe.beatPulse.load();
        float a = juce::jlimit(0.0f, 0.14f, (0.08f + 0.05f * beatPulse) * partyGlow);
        g.setColour(juce::Colour::fromHSV(partyHue, 0.55f, 0.9f, a));
        g.fillAll();
    }
}

void GlubGlubEditor::resized()
{
    auto b = getLocalBounds();
    drawer.setBounds(b.removeFromBottom(drawer.getCurrentHeight()));
    speech.setBounds(b.removeFromBottom(84));
    hype.setBounds(getWidth() - 182, 10, 170, 18);
    int discoW = juce::jlimit(120, 220, (int) (getWidth() * 0.32f));
    int discoH = (int) (discoW * 0.8f);
    disco.setBounds((int) (getWidth() * 0.5f) - discoW / 2, 0, discoW, discoH);
    fish.setBounds(b);
    bubbles.setBounds(b);
    shaker.setBounds(b);
    shaker.setWaterRect(b);
    // tuck the canister in the bottom-right corner of the tank
    shaker.setHome({ b.getRight() - 12 - shaker.GRID_W * shaker.CELL,
                     b.getBottom() - 10 - shaker.GRID_H * shaker.CELL });
}

void GlubGlubEditor::timerCallback()
{
    float energy = proc.vibe.energy.load();
    float bright = proc.vibe.brightness.load();
    float pulse = proc.vibe.beatPulse.load();
    float phase = proc.vibe.beatPhase.load();
    int inten = proc.vibe.intensity.load();
    int bar = proc.vibe.barCount.load();
    bool bubblesOn = proc.apvts.getRawParameterValue("bubblesOn")->load() > 0.5f;
    float speechRate = proc.apvts.getRawParameterValue("speechRate")->load();
    double now = juce::Time::getMillisecondCounterHiRes() / 1000.0 - startTime;
    const float dt = lastUpdateTime > 0.0 ? juce::jlimit(0.0f, 0.05f, (float) (now - lastUpdateTime)) : 1.0f / 60.0f;
    lastUpdateTime = now;

    // ---- feed boost (food shaker) ----
    shaker.update(juce::Time::getMillisecondCounterHiRes() / 1000.0);
    if (shaker.isFeeding())
        feedHoldUntil = juce::Time::getMillisecondCounterHiRes() / 1000.0 + 4.0;
    float feedTarget = juce::Time::getMillisecondCounterHiRes() / 1000.0 < feedHoldUntil ? 1.0f : 0.0f;
    float feed = proc.vibe.feedBoost.load();
    feed += (feedTarget - feed) * (feedTarget > feed ? 0.35f : 0.02f);
    if (feed < 0.001f) feed = 0.0f;
    proc.vibe.feedBoost.store(feed);

    const float hypeLevel = hypeEnvelope.update(proc.vibe.loudness.load(), pulse, feed, dt);
    fish.setGlassesOn(proc.apvts.getRawParameterValue("glassesOn")->load() > 0.5f);
    fish.setGentleMotion(proc.apvts.getRawParameterValue("gentleMotion")->load() > 0.5f);
    fish.setVibe(energy, bright, pulse, phase, inten, bar, feed, hypeLevel, dt, proc.vibe.bpm.load());

    hype.setHype(hypeLevel);
    disco.update(hypeLevel, now);

    if (phase >= 0.0f)
    {
        if (lastPhase >= 0.0f && phase < lastPhase - 0.5f)
            partyHue = std::fmod(partyHue + 0.27f, 1.0f);
    }
    else
    {
        partyHue = std::fmod(partyHue + 0.016f * (0.05f + pulse * 0.3f), 1.0f);
    }
    lastPhase = phase;

    float glowTarget = hypeLevel > 0.70f ? (hypeLevel - 0.70f) / 0.30f : 0.0f;
    partyGlow += (glowTarget - partyGlow) * 0.08f;

    // Ripple physics update
    for (auto& rip : ripples)
    {
        rip.radius += 1.8f;
        rip.alpha -= 0.032f;
    }
    ripples.erase(std::remove_if(ripples.begin(), ripples.end(),
        [](const Ripple& r) { return r.alpha <= 0.0f || r.radius > 70.0f; }), ripples.end());

    if (partyGlow > 0.004f || lastGlow > 0.004f || feed > 0.004f || disco.isShowing() || !ripples.empty())
        repaint();
    lastGlow = partyGlow;

    bubbles.setEnabled(bubblesOn);
    if (bubblesOn && inten == 2 && pulse > 0.75f)
        bubbles.burst(fish.getMouthPosition());
    bubbles.update(energy, fish.getMouthPosition());
    speech.update(now, energy, inten, speechRate, rng);
}

void GlubGlubEditor::mouseDown(const juce::MouseEvent& e)
{
    auto pos = e.position;
    ripples.push_back({ pos.x, pos.y, 2.0f, 0.9f });
    fish.triggerStartle(pos);
    bubbles.burst(fish.getMouthPosition());
    repaint();
}

void GlubGlubEditor::mouseMove(const juce::MouseEvent& e)
{
    fish.setMouseTarget(e.position, true);
}

void GlubGlubEditor::mouseDrag(const juce::MouseEvent& e)
{
    fish.setMouseTarget(e.position, true);
}

void GlubGlubEditor::mouseExit(const juce::MouseEvent&)
{
    fish.setMouseTarget({ -1.0f, -1.0f }, false);
}
