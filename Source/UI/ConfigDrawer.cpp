#include "ConfigDrawer.h"

ConfigDrawer::ConfigDrawer(juce::AudioProcessorValueTreeState& s) : state(s)
{
    showBtn.setButtonText(" settings ");
    showBtn.onClick = [this] { expanded = !expanded; setSize(getWidth(), expanded ? getExpandedHeight() : getCollapsedHeight()); resized(); };
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

    speechAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "speechRate", speechSlider);
    sensAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "sensitivity", sensSlider);
    hueAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, "hue", hueSlider);
    bubblesAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "bubblesOn", bubblesBtn);
}

void ConfigDrawer::resized()
{
    auto b = getLocalBounds();
    showBtn.setBounds(b.removeFromTop(28));
    if (!expanded) return;
    auto row = [&](juce::Label& l, juce::Slider& s)
    {
        auto r = b.removeFromTop(28);
        l.setBounds(r.removeFromLeft(70));
        s.setBounds(r);
    };
    row(speechLabel, speechSlider);
    row(sensLabel, sensSlider);
    row(hueLabel, hueSlider);
    bubblesBtn.setBounds(b.removeFromTop(24));
}
