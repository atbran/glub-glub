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

class GlubGlubEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    explicit GlubGlubEditor(GlubGlubProcessor&);
    ~GlubGlubEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    GlubGlubProcessor& proc;
    KoiFish fish;
    Bubbles bubbles;
    SpeechBox speech;
    ConfigDrawer drawer;
    HypeMeter hype;
    DiscoBall disco;
    juce::Random rng;
    double startTime = 0.0;
    float partyGlow = 0.0f;
    float partyHue = 0.55f;
    float lastPhase = -1.0f;
    float lastGlow = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlubGlubEditor)
};
