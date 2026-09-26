#include "ControlPanel.h"
#include "PixelLookAndFeel.h"

namespace
{
    const char* const kSkinNames[KoiFish::numSkins] = { "koi", "goldfish", "shubunkin", "neon", "golden" };
    const char* const kHatNames[3] = { "no hat", "party hat", "crown" };
}

ControlPanel::ControlPanel(juce::AudioProcessorValueTreeState& s, GlubProgress& p) : state(s), progress(p)
{
    const char* tabs[] = { "tank", "dance", "glub" };
    for (int i = 0; i < 3; ++i)
    {
        tabBtns[(size_t) i].setButtonText(tabs[i]);
        tabBtns[(size_t) i].onClick = [this, i] { showTab(static_cast<Tab>(i)); };
        addAndMakeVisible(tabBtns[(size_t) i]);
    }

    // ---- tank ----
    speechLabel.setText("speech", juce::dontSendNotification);
    sensLabel.setText("vibe", juce::dontSendNotification);
    hypeLabel.setText("hype", juce::dontSendNotification);
    hueLabel.setText("hue", juce::dontSendNotification);
    themeLabel.setText("tank", juce::dontSendNotification);
    for (auto* l : { &speechLabel, &sensLabel, &hypeLabel, &hueLabel, &themeLabel }) addChildComponent(l);

    speechSlider.setTooltip("Seconds between Glub's comments (30-90)");
    sensSlider.setTooltip("How easily the music gets him moving");
    hypeSlider.setTooltip("How quickly the HYPE meter fills (0.5x - 2x). Higher = the party starts sooner");
    hueSlider.setTooltip("Nudge the water colour");
    for (auto* sl : { &speechSlider, &sensSlider, &hypeSlider, &hueSlider })
    {
        sl->setSliderStyle(juce::Slider::LinearHorizontal);
        sl->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        addChildComponent(sl);
    }

    glassesBtn.setTooltip("Deal With It glasses");
    gentleBtn.setTooltip("Smaller movements without full spins or rolls");
    matesBtn.setTooltip("A school of neon tetras that scatter on kicks");
    for (auto* b : { &bubblesBtn, &glassesBtn, &gentleBtn, &matesBtn }) addChildComponent(b);

    speechAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "speechRate", speechSlider);
    sensAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "sensitivity", sensSlider);
    hypeAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "hypeSensitivity", hypeSlider);
    hueAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "hue", hueSlider);
    bubblesAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "bubblesOn", bubblesBtn);
    glassesAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "glassesOn", glassesBtn);
    gentleAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "gentleMotion", gentleBtn);
    matesAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "tankMates", matesBtn);

    const char* themes[] = { "lagoon", "midnight", "sunset" };
    for (int i = 0; i < 3; ++i)
    {
        auto& b = themeBtns[(size_t) i];
        b.setButtonText(themes[i]);
        b.onClick = [this, i]
        {
            if (auto* param = state.getParameter("theme"))
                param->setValueNotifyingHost(param->convertTo0to1((float) i));
        };
        addChildComponent(b);
    }

    // ---- dance ----
    danceLabel.setText("click a move:", juce::dontSendNotification);
    addChildComponent(danceLabel);
    wormBtn.setClickingTogglesState(true);
    wormBtn.setTooltip("He does the worm about five times as often, every bar");
    wormAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "wormMode", wormBtn);
    addChildComponent(wormBtn);

    const char* moves[KoiFish::numMoves] = { "worm", "roll", "spin", "flip", "shuffle", "head bop", "shimmy", "figure 8",
                                             "twerk", "breakdance", "loop", "moonwalk", "tail walk" };
    for (int i = 0; i < KoiFish::numMoves; ++i)
    {
        auto& b = moveBtns[(size_t) i];
        b.setButtonText(moves[i]);
        b.setTooltip("Queued moves start on the next downbeat");
        b.onClick = [this, i]
        {
            if (onMoveTriggered) onMoveTriggered(static_cast<KoiFish::MoveType>(i + 1));
        };
        addChildComponent(b);
    }

    // ---- glub ----
    skinLabel.setText("skin", juce::dontSendNotification);
    wearLabel.setText("wear", juce::dontSendNotification);
    addChildComponent(skinLabel);
    addChildComponent(wearLabel);
    for (int i = 0; i < KoiFish::numSkins; ++i)
    {
        skinBtns[(size_t) i].onClick = [this, i] { progress.setSkin(i); progress.save(); refreshWardrobe(); if (onWardrobeChanged) onWardrobeChanged(); };
        addChildComponent(skinBtns[(size_t) i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        hatBtns[(size_t) i].onClick = [this, i] { progress.setHat(i); progress.save(); refreshWardrobe(); if (onWardrobeChanged) onWardrobeChanged(); };
        addChildComponent(hatBtns[(size_t) i]);
    }
    chainBtn.onClick = [this] { progress.setChain(!progress.getChain()); progress.save(); refreshWardrobe(); if (onWardrobeChanged) onWardrobeChanged(); };
    addChildComponent(chainBtn);

    showTab(Tab::Tank);
}

void ControlPanel::showTab(Tab t)
{
    tab = t;
    for (int i = 0; i < 3; ++i)
        tabBtns[(size_t) i].setToggleState(static_cast<int>(t) == i, juce::dontSendNotification);

    const bool tank = t == Tab::Tank, dance = t == Tab::Dance, glub = t == Tab::Glub;
    for (juce::Component* c : { (juce::Component*) &speechLabel, (juce::Component*) &sensLabel, (juce::Component*) &hypeLabel,
                                (juce::Component*) &hueLabel, (juce::Component*) &themeLabel, (juce::Component*) &speechSlider,
                                (juce::Component*) &sensSlider, (juce::Component*) &hypeSlider, (juce::Component*) &hueSlider,
                                (juce::Component*) &bubblesBtn, (juce::Component*) &glassesBtn, (juce::Component*) &gentleBtn,
                                (juce::Component*) &matesBtn })
        c->setVisible(tank);
    for (auto& b : themeBtns) b.setVisible(tank);
    danceLabel.setVisible(dance);
    wormBtn.setVisible(dance);
    for (auto& b : moveBtns) b.setVisible(dance);
    skinLabel.setVisible(glub);
    wearLabel.setVisible(glub);
    for (auto& b : skinBtns) b.setVisible(glub);
    for (auto& b : hatBtns) b.setVisible(glub);
    chainBtn.setVisible(glub);
    refreshWardrobe();
    repaint();
}

void ControlPanel::refreshWardrobe()
{
    const int lvl = progress.getLevel();
    for (int i = 0; i < KoiFish::numSkins; ++i)
    {
        auto& b = skinBtns[(size_t) i];
        const int need = GlubProgress::levelRequiredForSkin(i);
        b.setEnabled(lvl >= need);
        b.setButtonText(lvl >= need ? juce::String(kSkinNames[i]) : "Lv " + juce::String(need));
        b.setTooltip(lvl >= need ? juce::String() : juce::String(kSkinNames[i]) + " unlocks at level " + juce::String(need));
        b.setToggleState(progress.getSkin() == i, juce::dontSendNotification);
    }
    for (int i = 0; i < 3; ++i)
    {
        auto& b = hatBtns[(size_t) i];
        const int need = GlubProgress::levelRequiredForHat(i);
        b.setEnabled(lvl >= need);
        b.setButtonText(lvl >= need ? juce::String(kHatNames[i]) : "Lv " + juce::String(need));
        b.setTooltip(lvl >= need ? juce::String() : juce::String(kHatNames[i]) + " unlocks at level " + juce::String(need));
        b.setToggleState(progress.getHat() == i, juce::dontSendNotification);
    }
    const int chainNeed = GlubProgress::levelRequiredForChain();
    chainBtn.setEnabled(lvl >= chainNeed);
    chainBtn.setButtonText(lvl >= chainNeed ? "gold chain" : "Lv " + juce::String(chainNeed));
    chainBtn.setToggleState(progress.getChain(), juce::dontSendNotification);
    shownLevel = lvl;
}

void ControlPanel::syncFromState(KoiFish::MoveType activeMove)
{
    const int theme = (int) state.getRawParameterValue("theme")->load();
    for (int i = 0; i < 3; ++i)
        themeBtns[(size_t) i].setToggleState(i == theme, juce::dontSendNotification);
    for (int i = 0; i < KoiFish::numMoves; ++i)
        moveBtns[(size_t) i].setToggleState(static_cast<int>(activeMove) == i + 1, juce::dontSendNotification);
    if (progress.getLevel() != shownLevel) refreshWardrobe();
    const int xpNow = (int) progress.getXp();
    if (tab == Tab::Glub && xpNow != shownXp) { shownXp = xpNow; repaint(); }
}

void ControlPanel::paint(juce::Graphics& g)
{
    auto b = getLocalBounds();
    g.setColour(juce::Colour(PixelLookAndFeel::ink));
    g.fillRect(b);
    g.setColour(juce::Colour(0xF0182430));
    g.fillRect(b.reduced(0, 3).withTrimmedBottom(-3));
    g.setColour(juce::Colour(PixelLookAndFeel::slateLight));
    for (int x = 0; x < getWidth(); x += 8)
        g.fillRect(x, (x / 8) % 2 == 0 ? 3 : 5, 8, 2);

    if (tab != Tab::Glub) return;

    // Level card: "LEVEL 4", an XP bar, and what the next level brings.
    auto area = getLocalBounds().reduced(12, 10).withTrimmedTop(34);
    auto header = area.removeFromTop(20);
    g.setFont(PixelLookAndFeel::pixelFont(15.0f));
    g.setColour(juce::Colour(0xFFFFC93C));
    g.drawText("LEVEL " + juce::String(progress.getLevel()), header, juce::Justification::centredLeft);
    g.setFont(PixelLookAndFeel::pixelFont(11.0f));
    g.setColour(juce::Colour(PixelLookAndFeel::cream).withAlpha(0.8f));
    g.drawText(juce::String((int) progress.getXpIntoLevel()) + " / " + juce::String((int) progress.getXpForThisLevel()) + " xp",
               header, juce::Justification::centredRight);

    auto bar = area.removeFromTop(12).reduced(0, 1);
    g.setColour(juce::Colour(PixelLookAndFeel::ink));
    g.fillRect(bar);
    auto inner = bar.reduced(2);
    g.setColour(juce::Colour(PixelLookAndFeel::slate));
    g.fillRect(inner);
    const float frac = juce::jlimit(0.0f, 1.0f, progress.getXpIntoLevel() / juce::jmax(1.0f, progress.getXpForThisLevel()));
    const int filled = (int) (inner.getWidth() * frac) / 4 * 4;
    g.setColour(juce::Colour(0xFFFFC93C));
    g.fillRect(inner.withWidth(filled));
    g.setColour(juce::Colour(0xFFFFF1A6));
    g.fillRect(inner.withWidth(filled).withHeight(2));

    juce::String next;
    for (int lvl = progress.getLevel() + 1; lvl <= 10 && next.isEmpty(); ++lvl)
        if (GlubProgress::unlockedAt(lvl).isNotEmpty())
            next = "next unlock: " + GlubProgress::unlockedAt(lvl) + " at level " + juce::String(lvl);
    if (next.isEmpty()) next = "every unlock earned. legendary glub.";
    g.setFont(PixelLookAndFeel::pixelFont(11.0f));
    g.setColour(juce::Colour(PixelLookAndFeel::cream).withAlpha(0.65f));
    g.drawText(next + "   (xp: dance, eat, get petted)", getLocalBounds().reduced(12, 8).removeFromBottom(16),
               juce::Justification::centredLeft);
}

void ControlPanel::resized()
{
    auto b = getLocalBounds().reduced(12, 10);
    b.removeFromTop(2);
    auto tabs = b.removeFromTop(24);
    const int tw = juce::jmin(110, tabs.getWidth() / 3);
    for (auto& t : tabBtns) t.setBounds(tabs.removeFromLeft(tw).reduced(2, 0));
    b.removeFromTop(8);

    // tank
    {
        auto r = b;
        auto sliderRow = [&](juce::Label& l, juce::Slider& s)
        {
            auto row = r.removeFromTop(24);
            l.setBounds(row.removeFromLeft(64));
            s.setBounds(row);
        };
        sliderRow(speechLabel, speechSlider);
        sliderRow(sensLabel, sensSlider);
        sliderRow(hypeLabel, hypeSlider);
        sliderRow(hueLabel, hueSlider);
        r.removeFromTop(4);
        auto themeRow = r.removeFromTop(24);
        themeLabel.setBounds(themeRow.removeFromLeft(64));
        const int w3 = themeRow.getWidth() / 3;
        for (auto& t : themeBtns) t.setBounds(themeRow.removeFromLeft(w3).reduced(2, 0));
        r.removeFromTop(4);
        auto toggles = r.removeFromTop(22);
        const int toggleW = toggles.getWidth() / 4;
        for (auto* t : { &bubblesBtn, &glassesBtn, &gentleBtn, &matesBtn }) t->setBounds(toggles.removeFromLeft(toggleW));
    }

    // dance
    {
        auto r = b;
        auto header = r.removeFromTop(26);
        wormBtn.setBounds(header.removeFromRight(140).reduced(2, 1));
        danceLabel.setBounds(header);
        r.removeFromTop(4);
        const int bw = r.getWidth() / 5;
        for (int row = 0; row * 5 < KoiFish::numMoves; ++row)
        {
            auto line = r.removeFromTop(26);
            for (int i = row * 5; i < juce::jmin(KoiFish::numMoves, row * 5 + 5); ++i)
                moveBtns[(size_t) i].setBounds(line.removeFromLeft(bw).reduced(2, 1));
        }
    }

    // glub: the level card is painted in the top 34 px
    {
        auto r = b;
        r.removeFromTop(38);
        auto skinRow = r.removeFromTop(26);
        skinLabel.setBounds(skinRow.removeFromLeft(48));
        const int sw = skinRow.getWidth() / KoiFish::numSkins;
        for (auto& s : skinBtns) s.setBounds(skinRow.removeFromLeft(sw).reduced(2, 1));
        r.removeFromTop(4);
        auto wearRow = r.removeFromTop(26);
        wearLabel.setBounds(wearRow.removeFromLeft(48));
        const int ww = wearRow.getWidth() / 4;
        for (auto& h : hatBtns) h.setBounds(wearRow.removeFromLeft(ww).reduced(2, 1));
        chainBtn.setBounds(wearRow.reduced(2, 1));
    }
}
