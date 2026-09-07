#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class ConfigDrawer : public juce::Component
{
public:
    ConfigDrawer(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
    int getCollapsedHeight() const { return 28; }
    int getExpandedHeight() const { return 120; }

private:
    juce::AudioProcessorValueTreeState& state;
    juce::ToggleButton showBtn { "gear" };
    juce::Slider speechSlider, sensSlider, hueSlider;
    juce::ToggleButton bubblesBtn { "bubbles" };
    juce::Label speechLabel, sensLabel, hueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speechAtt, sensAtt, hueAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bubblesAtt;
    bool expanded = false;
};
