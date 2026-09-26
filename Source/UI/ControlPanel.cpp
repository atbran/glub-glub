#include "ControlPanel.h"
#include "PixelLookAndFeel.h"

ControlPanel::ControlPanel(juce::AudioProcessorValueTreeState& s) : state(s)
{
    speechLabel.setText("speech", juce::dontSendNotification);
    sensLabel.setText("vibe", juce::dontSendNotification);
    hueLabel.setText("hue", juce::dontSendNotification);
    themeLabel.setText("tank", juce::dontSendNotification);
    danceLabel.setText("dance! (click a move)", juce::dontSendNotification);
    for (auto* l : { &speechLabel, &sensLabel, &hueLabel, &themeLabel, &danceLabel }) addAndMakeVisible(l);

    speechSlider.setTooltip("Seconds between Glub's comments (30-90)");
    sensSlider.setTooltip("How easily the music gets him moving");
    hueSlider.setTooltip("Nudge the water colour");
    for (auto* sl : { &speechSlider, &sensSlider, &hueSlider })
    {
        sl->setSliderStyle(juce::Slider::LinearHorizontal);
        sl->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        addAndMakeVisible(sl);
    }

    glassesBtn.setTooltip("Deal With It glasses");
    gentleBtn.setTooltip("Smaller movements without full spins or rolls");
    matesBtn.setTooltip("A school of neon tetras that scatter on kicks");
    for (auto* b : { &bubblesBtn, &glassesBtn, &gentleBtn, &matesBtn }) addAndMakeVisible(b);

    speechAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "speechRate", speechSlider);
    sensAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "sensitivity", sensSlider);
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
        b.setClickingTogglesState(false);
        b.onClick = [this, i]
        {
            if (auto* p = state.getParameter("theme"))
                p->setValueNotifyingHost(p->convertTo0to1((float) i));
        };
        addAndMakeVisible(b);
    }

    const char* moves[] = { "worm", "roll", "spin", "flip", "shuffle", "head bop", "shimmy", "figure 8", "twerk", "breakdance" };
    for (int i = 0; i < 10; ++i)
    {
        auto& b = moveBtns[(size_t) i];
        b.setButtonText(moves[i]);
        b.setTooltip("Queued moves start on the next downbeat");
        b.onClick = [this, i]
        {
            if (onMoveTriggered) onMoveTriggered(static_cast<KoiFish::MoveType>(i + 1));
        };
        addAndMakeVisible(b);
    }
}

void ControlPanel::syncFromState(KoiFish::MoveType activeMove)
{
    const int theme = (int) state.getRawParameterValue("theme")->load();
    for (int i = 0; i < 3; ++i)
        themeBtns[(size_t) i].setToggleState(i == theme, juce::dontSendNotification);
    for (int i = 0; i < 10; ++i)
        moveBtns[(size_t) i].setToggleState(static_cast<int>(activeMove) == i + 1, juce::dontSendNotification);
}

void ControlPanel::paint(juce::Graphics& g)
{
    auto b = getLocalBounds();
    g.setColour(juce::Colour(PixelLookAndFeel::ink));
    g.fillRect(b);
    g.setColour(juce::Colour(0xF0182430));
    g.fillRect(b.reduced(0, 3).withTrimmedBottom(-3));
    // Pixel "wave" trim along the top edge.
    g.setColour(juce::Colour(PixelLookAndFeel::slateLight));
    for (int x = 0; x < getWidth(); x += 8)
        g.fillRect(x, (x / 8) % 2 == 0 ? 3 : 5, 8, 2);
}

void ControlPanel::resized()
{
    auto b = getLocalBounds().reduced(12, 10);
    b.removeFromTop(4);
    auto sliderRow = [&](juce::Label& l, juce::Slider& s)
    {
        auto r = b.removeFromTop(24);
        l.setBounds(r.removeFromLeft(64));
        s.setBounds(r);
    };
    sliderRow(speechLabel, speechSlider);
    sliderRow(sensLabel, sensSlider);
    sliderRow(hueLabel, hueSlider);

    b.removeFromTop(4);
    auto themeRow = b.removeFromTop(24);
    themeLabel.setBounds(themeRow.removeFromLeft(64));
    const int tw = themeRow.getWidth() / 3;
    for (auto& t : themeBtns) t.setBounds(themeRow.removeFromLeft(tw).reduced(2, 0));

    b.removeFromTop(4);
    auto toggles = b.removeFromTop(22);
    const int toggleW = toggles.getWidth() / 4;
    for (auto* t : { &bubblesBtn, &glassesBtn, &gentleBtn, &matesBtn }) t->setBounds(toggles.removeFromLeft(toggleW));

    b.removeFromTop(4);
    danceLabel.setBounds(b.removeFromTop(18));
    for (int row = 0; row < 2; ++row)
    {
        auto r = b.removeFromTop(26);
        const int bw = r.getWidth() / 5;
        for (int i = 0; i < 5; ++i)
            moveBtns[(size_t) (row * 5 + i)].setBounds(r.removeFromLeft(bw).reduced(2, 1));
    }
}
