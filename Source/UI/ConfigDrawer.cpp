#include "ConfigDrawer.h"

ConfigDrawer::ConfigDrawer(juce::AudioProcessorValueTreeState& s) : state(s)
{
    showBtn.setButtonText(" settings ");
    showBtn.onClick = [this]
    {
        expanded = !expanded;
        setControlsExpanded(expanded);
        resized();
        if (onHeightChanged != nullptr)
            onHeightChanged();
    };
    addAndMakeVisible(showBtn);

    speechLabel.setText("speech (s)", juce::dontSendNotification);
    sensLabel.setText("vibe", juce::dontSendNotification);
    hueLabel.setText("hue", juce::dontSendNotification);
    for (auto* l : { &speechLabel, &sensLabel, &hueLabel }) { l->setFont(juce::Font(12.0f)); addAndMakeVisible(l); }

    speechSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    sensSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    hueSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    for (auto* sl : { &speechSlider, &sensSlider, &hueSlider }) addAndMakeVisible(sl);

    bubblesBtn.setButtonText("bubbles");
    addAndMakeVisible(bubblesBtn);
    addAndMakeVisible(glassesBtn);
    addAndMakeVisible(gentleBtn);
    glassesBtn.setTooltip("Keep the Deal With It glasses on or take them off");
    gentleBtn.setTooltip("Smaller movements without full spins or rolls");

    speechAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "speechRate", speechSlider);
    sensAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "sensitivity", sensSlider);
    hueAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "hue", hueSlider);
    bubblesAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "bubblesOn", bubblesBtn);
    glassesAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "glassesOn", glassesBtn);
    gentleAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "gentleMotion", gentleBtn);

    const char* names[] = { "The Worm", "Barrel Roll", "Spin", "Flip", "Shuffle", "Head Bop", "Tail Shimmy", "Figure Eight", "Twerk" };
    for (int i = 0; i < 9; ++i) movePicker.addItem(names[i], i + 1);
    movePicker.setSelectedId(1, juce::dontSendNotification);
    movePicker.setTooltip("Choose a move, then press Dance. A new move waits for the current move to finish.");
    addAndMakeVisible(movePicker);
    danceBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF243242));
    danceBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFFDF6E3));
    addAndMakeVisible(danceBtn);
    danceBtn.onClick = [this]
    {
        if (onMoveTriggered) onMoveTriggered(static_cast<KoiFish::MoveType>(movePicker.getSelectedId()));
    };

    setControlsExpanded(false);
}

void ConfigDrawer::setControlsExpanded(bool on)
{
    speechSlider.setVisible(on);
    sensSlider.setVisible(on);
    hueSlider.setVisible(on);
    speechLabel.setVisible(on);
    sensLabel.setVisible(on);
    hueLabel.setVisible(on);
    bubblesBtn.setVisible(on);
    glassesBtn.setVisible(on);
    gentleBtn.setVisible(on);
    movePicker.setVisible(on);
    danceBtn.setVisible(on);
}

void ConfigDrawer::resized()
{
    auto b = getLocalBounds();
    showBtn.setBounds(b.removeFromTop(28));
    if (!expanded) return;
    auto row = [&](juce::Label& l, juce::Slider& s)
    {
        auto r = b.removeFromTop(24);
        l.setBounds(r.removeFromLeft(70));
        s.setBounds(r);
    };
    row(speechLabel, speechSlider);
    row(sensLabel, sensSlider);
    row(hueLabel, hueSlider);
    auto toggles = b.removeFromTop(22);
    const int toggleWidth = toggles.getWidth() / 3;
    bubblesBtn.setBounds(toggles.removeFromLeft(toggleWidth));
    glassesBtn.setBounds(toggles.removeFromLeft(toggleWidth));
    gentleBtn.setBounds(toggles);

    auto r = b.removeFromTop(26).reduced(2, 1);
    danceBtn.setBounds(r.removeFromRight(80).reduced(2, 0));
    movePicker.setBounds(r.reduced(2, 0));
}
