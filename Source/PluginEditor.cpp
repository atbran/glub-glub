#include "PluginEditor.h"

GlubGlubEditor::GlubGlubEditor(GlubGlubProcessor& p)
    : AudioProcessorEditor(p), proc(p), panel(p.apvts, *progress)
{
    setLookAndFeel(&lnf);
    setSize(500, 500);
    setResizable(true, true);
    setResizeLimits(360, 360, 1024, 1024);

    addAndMakeVisible(world);
    world.setInterceptsMouseClicks(false, true);
    world.addAndMakeVisible(scene);
    world.addAndMakeVisible(fish);
    world.addAndMakeVisible(bubbles);
    world.addAndMakeVisible(disco);
    world.addAndMakeVisible(shaker);
    addAndMakeVisible(speech);
    addAndMakeVisible(hype);
    addChildComponent(panel);
    addAndMakeVisible(tankBtn);

    for (juce::Component* c : { (juce::Component*) &fish, (juce::Component*) &bubbles, (juce::Component*) &speech, (juce::Component*) &disco })
        c->setInterceptsMouseClicks(false, false);

    panel.onMoveTriggered = [this](KoiFish::MoveType m) { fish.triggerMove(m); };
    seenLevelUps = progress->getLevelUpSerial();
    lastWormMode = p.apvts.getRawParameterValue("wormMode")->load() > 0.5f;
    tankBtn.setTooltip("Tank settings and dance moves");
    tankBtn.onClick = [this] { panelWanted = !panelWanted; };

    startTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    startTimerHz(60);
}

GlubGlubEditor::~GlubGlubEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void GlubGlubEditor::paint(juce::Graphics& g)
{
    // Bottom bar: the tank's wooden stand.
    auto bar = getLocalBounds().removeFromBottom(barHeight);
    g.setColour(juce::Colour(PixelLookAndFeel::ink));
    g.fillRect(bar);
    g.setColour(juce::Colour(0xFF4A2E1E));
    g.fillRect(bar.reduced(0, 3).withTrimmedTop(1));
    g.setColour(juce::Colour(0xFF6B4530));
    for (int x = 0; x < bar.getWidth(); x += 24)
        g.fillRect(x + 6, bar.getY() + 8 + (x / 24) % 2 * 6, 10, 2);
    g.setFont(PixelLookAndFeel::pixelFont(11.0f));
    g.setColour(juce::Colour(PixelLookAndFeel::cream).withAlpha(0.7f));
    g.drawText("glub-glub " JucePlugin_VersionString, bar.reduced(10, 0), juce::Justification::centredRight);

    // Level badge: "Lv 3" and a chunky XP bar in the middle of the stand.
    if (!levelBadgeArea.isEmpty())
    {
        auto badge = levelBadgeArea;
        g.setFont(PixelLookAndFeel::pixelFont(12.0f));
        g.setColour(juce::Colour(0xFFFFC93C));
        g.drawText("Lv " + juce::String(progress->getLevel()), badge.removeFromLeft(44), juce::Justification::centredLeft);
        auto xpBar = badge.withSizeKeepingCentre(badge.getWidth(), 10);
        g.setColour(juce::Colour(PixelLookAndFeel::ink));
        g.fillRect(xpBar);
        auto inner = xpBar.reduced(2);
        g.setColour(juce::Colour(0xFF2A1A10));
        g.fillRect(inner);
        const float frac = juce::jlimit(0.0f, 1.0f, progress->getXpIntoLevel() / juce::jmax(1.0f, progress->getXpForThisLevel()));
        g.setColour(juce::Colour(0xFFFFC93C));
        g.fillRect(inner.withWidth((int) (inner.getWidth() * frac) / 3 * 3));
    }
}

void GlubGlubEditor::resized()
{
    auto b = getLocalBounds();
    auto bar = b.removeFromBottom(barHeight);
    tankBtn.setBounds(bar.removeFromLeft(78).reduced(4, 3));
    levelBadgeArea = bar.withSizeKeepingCentre(juce::jmin(170, bar.getWidth() - 200), bar.getHeight()).reduced(0, 4);

    world.setBounds(b); // at the origin, so world and editor coordinates match unzoomed
    scene.setBounds(b);
    auto swim = b.withTrimmedBottom(speechHeight);
    speech.setBounds(b.getX(), swim.getBottom(), b.getWidth(), speechHeight);
    fish.setBounds(swim);
    bubbles.setBounds(swim);
    shaker.setBounds(swim);
    shaker.setWaterRect(swim);
    // Pellets settle a tenth of the way up the swim area; the sand starts there.
    scene.setFloorY(swim.getBottom() - swim.getHeight() * 0.1f + 8.0f);
    shaker.setHome({ swim.getRight() - 12 - FoodShaker::GRID_W * FoodShaker::CELL,
                     swim.getBottom() - 10 - FoodShaker::GRID_H * FoodShaker::CELL });

    hype.setBounds(getWidth() - 182, 10, 170, 36);
    const int discoW = juce::jlimit(120, 220, (int) (getWidth() * 0.32f));
    disco.setBounds(getWidth() / 2 - discoW / 2, 0, discoW, (int) (discoW * 0.8f));
    layoutPanel();
}

void GlubGlubEditor::layoutPanel()
{
    const int visible = juce::roundToInt(panelOpen * ControlPanel::panelHeight);
    panel.setBounds(0, getHeight() - barHeight - visible, getWidth(), ControlPanel::panelHeight);
    panel.setVisible(visible > 0);
}

bool GlubGlubEditor::trySpeak(const juce::String& line, double now, double cooldown)
{
    if (now - lastSpokeAt < cooldown || speech.hasText()) return false;
    speech.shout(line, now);
    lastSpokeAt = now;
    return true;
}

void GlubGlubEditor::timerCallback()
{
    auto& vibe = proc.vibe;
    const float energy = vibe.energy.load();
    const float bright = vibe.brightness.load();
    const float pulse = vibe.beatPulse.load();
    const float phase = vibe.beatPhase.load();
    const float bpm = vibe.bpm.load();
    const int inten = vibe.intensity.load();
    const int bar = vibe.barCount.load();
    const int tempoSource = vibe.tempoSource.load();
    auto param = [this](const char* id) { return proc.apvts.getRawParameterValue(id)->load(); };
    const bool bubblesOn = param("bubblesOn") > 0.5f;

    const double absNow = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    const double now = absNow - startTime;
    const float dt = lastUpdateTime > 0.0 ? juce::jlimit(0.0f, 0.05f, (float) (now - lastUpdateTime)) : 1.0f / 60.0f;
    lastUpdateTime = now;

    // ---- feeding: shaking food boosts the party, the koi eats what lands ----
    shaker.update(absNow);
    if (shaker.isFeeding())
        feedHoldUntil = absNow + 4.0;
    const float feedTarget = absNow < feedHoldUntil ? 1.0f : 0.0f;
    float feed = vibe.feedBoost.load();
    feed += (feedTarget - feed) * (feedTarget > feed ? 0.35f : 0.02f);
    if (feed < 0.001f) feed = 0.0f;
    vibe.feedBoost.store(feed);

    const auto mouth = fish.getMouthPosition();
    fish.setFoodTarget(shaker.nearestPellet(mouth));
    const float biteRadius = juce::jmax(10.0f, fish.getHeight() / 26.0f);
    if (shaker.eatPelletNear(mouth, biteRadius))
    {
        fish.chomp();
        progress->addXp(3.0f);
        if (bubblesOn) bubbles.burst(mouth);
        ++bitesSinceSpeech;
        static const char* nomLines[] = { "nom.", "NOM NOM NOM", "5 stars. would glub again.", "flakes?! for ME?",
                                          "chef's kiss (I have no hands)", "tastes like bass." };
        if (fish.getFullness() > 0.8f)
            trySpeak("I am SO full. do not feed me. (feed me)", now, 20.0);
        else if (bitesSinceSpeech >= 3 && trySpeak(nomLines[rng.nextInt(6)], now))
            bitesSinceSpeech = 0;
    }

    const float hypeLevel = hypeEnvelope.update(vibe.loudness.load(), pulse, feed, dt, param("hypeSensitivity"));
    fish.setGlassesOn(param("glassesOn") > 0.5f);
    fish.setGentleMotion(param("gentleMotion") > 0.5f);
    fish.setSkin(progress->getSkin());
    fish.setHat(progress->getHat());
    fish.setChain(progress->getChain());

    // Worm mode: flipping it on gets an instant worm and an announcement.
    const bool wormMode = param("wormMode") > 0.5f;
    fish.setWormMode(wormMode);
    if (wormMode && !lastWormMode)
    {
        fish.triggerMove(KoiFish::MoveType::Worm);
        speech.shout("WORM MODE ACTIVATED. glub.", now);
        lastSpokeAt = now;
    }
    lastWormMode = wormMode;
    fish.setVibe(energy, bright, pulse, phase, inten, bar, feed, hypeLevel, dt, bpm);

    if (fish.consumeBreakdanceTriggered())
    {
        static const char* bdShouts[] = { "BREAK IT DOWN!!", "HEADSPIN GLUB!!", "FREEZE!!",
                                          "DROP THE BASS, GLUB!!", "WINDMILL POWER!!", "B-BOY KOI IN THE TANK!!" };
        speech.shout(bdShouts[rng.nextInt(6)], now);
        lastSpokeAt = now;
        bubbles.vortex(fish.getFloorContactPos());
        progress->addXp(20.0f);
    }

    // Glub brags when he finds the beat on his own.
    if (tempoSource == VibeState::tempoDetected && lastTempoSource == VibeState::tempoNone)
    {
        const int rounded = juce::roundToInt(bpm);
        const juce::String lines[] = { "ooh, " + juce::String(rounded) + " BPM. my fins know this one.",
                                       juce::String(rounded) + " BPM? no DAW needed, I have ears.",
                                       "locking in at " + juce::String(rounded) + " BPM. glub glub glub." };
        trySpeak(lines[rng.nextInt(3)], now, 25.0);
    }
    lastTempoSource = tempoSource;

    hype.setHype(hypeLevel);
    hype.setTempo(bpm, tempoSource, phase);
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
    const float glowTarget = hypeLevel > 0.70f ? (hypeLevel - 0.70f) / 0.30f : 0.0f;
    partyGlow += (glowTarget - partyGlow) * 0.08f;

    TankScene::Frame frame;
    frame.low = vibe.low.load();
    frame.mid = vibe.mid.load();
    frame.high = vibe.high.load();
    frame.kick = vibe.kick.load();
    frame.pulse = pulse;
    frame.partyGlow = partyGlow;
    frame.partyHue = partyHue;
    frame.discoAppear = disco.getAppear();
    frame.discoCentre = disco.getBounds().getTopLeft().toFloat() + disco.getBallCentre();
    frame.koi = { fish.getX() + fish.getWidth() * 0.5f, fish.getY() + fish.getHeight() * 0.52f };
    frame.tankMates = param("tankMates") > 0.5f;
    scene.setTheme(static_cast<TankScene::Theme>(juce::jlimit(0, 2, (int) param("theme"))), param("hue"));
    scene.update(frame, now, dt);
    updateCamera(hypeLevel, phase, dt);

    // ---- XP: dancing to real music (faster when hyped), petting, big moves ----
    if (energy > 0.15f)
        progress->addXp(dt * (0.6f + 1.4f * hypeLevel));
    if (fish.isBeingPetted())
        progress->addXp(dt);
    if (fish.consumePetStarted())
    {
        static const char* petLines[] = { "hehe... that tickles.", "scritches accepted.", "I am a good fish. confirmed.",
                                          "don't stop. ever.", "purr? glub. same thing.", "10/10 petting technique." };
        trySpeak(petLines[rng.nextInt(6)], now, 8.0);
    }
    if (progress->getLevelUpSerial() != seenLevelUps)
    {
        seenLevelUps = progress->getLevelUpSerial();
        const int lvl = progress->getLevel();
        const auto unlocked = GlubProgress::unlockedAt(lvl);
        speech.shout("LEVEL UP! level " + juce::String(lvl) + (unlocked.isNotEmpty() ? " - unlocked " + unlocked + "!" : "!"), now);
        lastSpokeAt = now;
        bubbles.confetti({ fish.getWidth() * 0.5f, fish.getHeight() * 0.45f });
        repaint(levelBadgeArea);
    }
    if ((int) progress->getXp() != shownXp)
    {
        shownXp = (int) progress->getXp();
        repaint(levelBadgeArea);
    }

    bubbles.setEnabled(bubblesOn);
    if (bubblesOn && scene.consumeChestBurp())
        bubbles.burst(scene.getChestMouth());
    if (bubblesOn && inten == 2 && pulse > 0.75f)
        bubbles.burst(mouth);
    if (bubblesOn && fish.isBreakdancing() && rng.nextFloat() < 0.25f)
        bubbles.vortex(fish.getFloorContactPos());
    bubbles.update(energy, mouth);
    speech.update(now, energy, inten, param("speechRate"), rng);

    // Panel slide: a quick ease-out, no overshoot.
    const float target = panelWanted ? 1.0f : 0.0f;
    if (std::abs(target - panelOpen) > 0.001f)
    {
        panelOpen += (target - panelOpen) * (1.0f - std::exp(-dt / 0.07f));
        if (std::abs(target - panelOpen) < 0.004f) panelOpen = target;
        layoutPanel();
    }
    tankBtn.setButtonText(panelWanted ? "close" : "tank");
    tankBtn.setToggleState(panelWanted, juce::dontSendNotification);
    if (panel.isVisible())
        panel.syncFromState(fish.getActiveMove());
}

void GlubGlubEditor::mouseDown(const juce::MouseEvent& e)
{
    if (panelWanted)
    {
        panelWanted = false; // Clicking the water closes the panel.
        return;
    }
    scene.addRipple(scene.getLocalPoint(this, e.position));
    fish.triggerStartle(fish.getLocalPoint(this, e.position));
    bubbles.burst(fish.getMouthPosition());
}

void GlubGlubEditor::mouseMove(const juce::MouseEvent& e)
{
    fish.setMouseTarget(fish.getLocalPoint(this, e.position), true);
}

void GlubGlubEditor::mouseDrag(const juce::MouseEvent& e)
{
    fish.setMouseTarget(fish.getLocalPoint(this, e.position), true);
}

void GlubGlubEditor::updateCamera(float hypeLevel, float beatPhase, float dt)
{
    // Fades in only when hype is near the top, and never in gentle mode.
    const bool gentle = proc.apvts.getRawParameterValue("gentleMotion")->load() > 0.5f;
    const float target = gentle ? 0.0f : juce::jlimit(0.0f, 1.0f, (hypeLevel - 0.80f) / 0.12f);
    cameraAmount += (target - cameraAmount) * (1.0f - std::exp(-dt / 0.8f));

    // Punch in on each beat over a few milliseconds, then ease back out:
    // continuous across the wrap, so the camera never pops.
    float punch = 0.0f;
    if (beatPhase >= 0.0f)
        punch = beatPhase < 0.06f ? juce::jmap(beatPhase, 0.0f, 0.06f, 0.0f, 1.0f)
                                  : std::pow(1.0f - (beatPhase - 0.06f) / 0.94f, 3.0f);
    const float zoom = 1.0f + cameraAmount * (0.025f + 0.035f * punch); // at most ~6%

    // Frame Glub, leaning toward the tank centre so the camera doesn't chase every move.
    const auto tankCentre = world.getLocalBounds().toFloat().getCentre();
    const auto target2 = tankCentre + (fish.getBodyCentre() + fish.getPosition().toFloat() - tankCentre) * 0.6f;
    if (cameraFocus.isOrigin()) cameraFocus = target2;
    cameraFocus += (target2 - cameraFocus) * (1.0f - std::exp(-dt / 0.35f));

    if (std::abs(zoom - cameraZoom) < 0.0005f && zoom > 1.0005f == cameraZoom > 1.0005f) return;
    cameraZoom = zoom;
    const auto transform = cameraZoom > 1.0005f
        ? juce::AffineTransform::scale(cameraZoom, cameraZoom, cameraFocus.x, cameraFocus.y)
        : juce::AffineTransform();
    for (juce::Component* c : { (juce::Component*) &scene, (juce::Component*) &fish, (juce::Component*) &bubbles,
                                (juce::Component*) &disco, (juce::Component*) &shaker })
        c->setTransform(transform);
}

void GlubGlubEditor::mouseExit(const juce::MouseEvent&)
{
    fish.setMouseTarget({ -1.0f, -1.0f }, false);
}
