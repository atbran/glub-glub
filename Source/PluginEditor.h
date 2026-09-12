#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "UI/KoiFish.h"
#include "UI/Bubbles.h"
#include "UI/SpeechBox.h"
#include "UI/ConfigDrawer.h"
#include "UI/HypeMeter.h"
#include "UI/DiscoBall.h"
#include "UI/FoodShaker.h"
#include "UI/HypeEnvelope.h"

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
    struct Ripple
    {
        float x = 0.0f;
        float y = 0.0f;
        float radius = 2.0f;
        float alpha = 0.85f;
    };
    std::vector<Ripple> ripples;

    GlubGlubProcessor& proc;
    KoiFish fish;
    Bubbles bubbles;
    SpeechBox speech;
    ConfigDrawer drawer;
    HypeMeter hype;
    DiscoBall disco;
    FoodShaker shaker;
    juce::Random rng;
    double startTime = 0.0;
    double feedHoldUntil = 0.0;
    float partyGlow = 0.0f;
    float partyHue = 0.55f;
    float lastPhase = -1.0f;
    float lastGlow = 0.0f;
    HypeEnvelope hypeEnvelope;
    double lastUpdateTime = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlubGlubEditor)
};
