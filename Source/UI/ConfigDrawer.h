#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "KoiFish.h"
#include <functional>

class ConfigDrawer : public juce::Component
{
public:
    ConfigDrawer(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
    int getCollapsedHeight() const { return 28; }
    int getExpandedHeight() const { return 152; }
    int getCurrentHeight() const { return expanded ? getExpandedHeight() : getCollapsedHeight(); }

    std::function<void()> onHeightChanged;
    std::function<void(KoiFish::MoveType)> onMoveTriggered;

private:
    void setControlsExpanded(bool on);

    juce::AudioProcessorValueTreeState& state;
    juce::ToggleButton showBtn { "gear" };
    juce::Slider speechSlider, sensSlider, hueSlider;
    juce::ToggleButton bubblesBtn { "bubbles" };
    juce::ToggleButton glassesBtn { "glasses" }, gentleBtn { "gentle" };
    juce::Label speechLabel, sensLabel, hueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speechAtt, sensAtt, hueAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bubblesAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> glassesAtt, gentleAtt;

    juce::ComboBox movePicker;
    juce::TextButton danceBtn { "Dance" };

    bool expanded = false;
};
