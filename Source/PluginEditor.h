#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "UI/KoiFish.h"
#include "UI/Bubbles.h"
#include "UI/SpeechBox.h"
#include "UI/ControlPanel.h"
#include "UI/HypeMeter.h"
#include "UI/DiscoBall.h"
#include "UI/FoodShaker.h"
#include "UI/HypeEnvelope.h"
#include "UI/TankScene.h"
#include "UI/PixelLookAndFeel.h"
#include "UI/GlubProgress.h"

class GlubGlubEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    explicit GlubGlubEditor(GlubGlubProcessor&);
    ~GlubGlubEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

private:
    friend struct EditorShots;
    static constexpr int barHeight = 28;
    static constexpr int speechHeight = 84;

    void layoutPanel();
    bool trySpeak(const juce::String& line, double now, double cooldown = 14.0);

    GlubGlubProcessor& proc;
    juce::SharedResourcePointer<GlubProgress> progress; // one pet shared by every instance
    PixelLookAndFeel lnf;
    juce::TooltipWindow tooltips { this, 700 };
    // Everything the camera sees; the HUD (speech, meter, panel) stays outside.
    juce::Component world;
    TankScene scene;
    KoiFish fish;
    Bubbles bubbles;
    SpeechBox speech;
    ControlPanel panel;
    juce::TextButton tankBtn { "tank" };
    HypeMeter hype;
    DiscoBall disco;
    FoodShaker shaker;
    juce::Random rng;

    double startTime = 0.0;
    double feedHoldUntil = 0.0;
    double lastSpokeAt = -100.0;
    double lastUpdateTime = 0.0;
    float partyGlow = 0.0f;
    float partyHue = 0.55f;
    float lastPhase = -1.0f;
    float panelOpen = 0.0f;
    bool panelWanted = false;
    int lastTempoSource = 0;
    int bitesSinceSpeech = 0;
    int seenLevelUps = 0, shownXp = -1;
    bool lastWormMode = false;
    juce::Rectangle<int> levelBadgeArea;

    // Beat-synced camera punch at max hype: gentle, just enough to feel it.
    void updateCamera(float hypeLevel, float beatPhase, float dt);
    float cameraAmount = 0.0f, cameraZoom = 1.0f;
    juce::Point<float> cameraFocus;
    HypeEnvelope hypeEnvelope;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlubGlubEditor)
};
