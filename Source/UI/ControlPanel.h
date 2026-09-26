#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "KoiFish.h"
#include <array>
#include <functional>

// Settings panel that slides up over the tank instead of squashing it.
// The editor owns the open/close animation and positions the panel each frame.
class ControlPanel : public juce::Component
{
public:
    explicit ControlPanel(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
    void paint(juce::Graphics& g) override;

    static constexpr int panelHeight = 254;
    void syncFromState(KoiFish::MoveType activeMove);

    std::function<void(KoiFish::MoveType)> onMoveTriggered;

private:
    juce::AudioProcessorValueTreeState& state;
    juce::Slider speechSlider, sensSlider, hueSlider;
    juce::Label speechLabel, sensLabel, hueLabel, danceLabel, themeLabel;
    juce::ToggleButton bubblesBtn { "bubbles" }, glassesBtn { "shades" }, gentleBtn { "gentle" }, matesBtn { "tank mates" };
    std::array<juce::TextButton, 3> themeBtns;
    std::array<juce::TextButton, KoiFish::numMoves> moveBtns;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speechAtt, sensAtt, hueAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bubblesAtt, glassesAtt, gentleAtt, matesAtt;
};
