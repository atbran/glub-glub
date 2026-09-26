#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "KoiFish.h"
#include "GlubProgress.h"
#include <array>
#include <functional>

// Settings panel that slides up over the tank instead of squashing it.
// Three tabs: tank (look + feel), dance (moves + worm mode), glub (level + wardrobe).
// The editor owns the open/close animation and positions the panel each frame.
class ControlPanel : public juce::Component
{
public:
    ControlPanel(juce::AudioProcessorValueTreeState& apvts, GlubProgress& progress);
    void resized() override;
    void paint(juce::Graphics& g) override;

    static constexpr int panelHeight = 206;
    void syncFromState(KoiFish::MoveType activeMove);

    std::function<void(KoiFish::MoveType)> onMoveTriggered;
    std::function<void()> onWardrobeChanged;

private:
    enum class Tab { Tank = 0, Dance, Glub };
    void showTab(Tab t);
    void refreshWardrobe();

    juce::AudioProcessorValueTreeState& state;
    GlubProgress& progress;
    Tab tab = Tab::Tank;
    std::array<juce::TextButton, 3> tabBtns;

    // tank
    juce::Slider speechSlider, sensSlider, hypeSlider, hueSlider;
    juce::Label speechLabel, sensLabel, hypeLabel, hueLabel, themeLabel;
    juce::ToggleButton bubblesBtn { "bubbles" }, glassesBtn { "shades" }, gentleBtn { "gentle" }, matesBtn { "tank mates" };
    std::array<juce::TextButton, 3> themeBtns;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speechAtt, sensAtt, hypeAtt, hueAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bubblesAtt, glassesAtt, gentleAtt, matesAtt;

    // dance
    juce::Label danceLabel;
    juce::TextButton wormBtn { "WORM MODE" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> wormAtt;
    std::array<juce::TextButton, KoiFish::numMoves> moveBtns;

    // glub
    juce::Label skinLabel, wearLabel;
    std::array<juce::TextButton, KoiFish::numSkins> skinBtns;
    std::array<juce::TextButton, 3> hatBtns;
    juce::TextButton chainBtn { "chain" };
    int shownLevel = -1, shownXp = -1;
};
