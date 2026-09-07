#include "PluginEditor.h"

GlubGlubEditor::GlubGlubEditor(GlubGlubProcessor& p)
    : AudioProcessorEditor(p), proc(p), drawer(p.apvts)
{
    setSize(500, 500);
    setResizable(true, true);
    setResizeLimits(320, 320, 1024, 1024);
    addAndMakeVisible(fish);
    addAndMakeVisible(bubbles);
    addAndMakeVisible(speech);
    addAndMakeVisible(drawer);
    drawer.onHeightChanged = [this] { resized(); };
    bubbles.setInterceptsMouseClicks(false, false);
    speech.setInterceptsMouseClicks(false, false);
    startTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    startTimerHz(60);
}

GlubGlubEditor::~GlubGlubEditor() { stopTimer(); }

void GlubGlubEditor::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    float hueShift = proc.apvts.getRawParameterValue("hue")->load();
    juce::Colour top = juce::Colour::fromHSV(0.58f + hueShift, 0.55f, 0.35f, 1.0f);
    juce::Colour bottom = juce::Colour::fromHSV(0.60f + hueShift, 0.6f, 0.12f, 1.0f);
    g.setGradientFill(juce::ColourGradient(top, 0, 0, bottom, 0, b.getHeight(), false));
    g.fillAll();
    // pebbles
    g.setColour(juce::Colour(0xFF2A3B4D));
    for (int i = 0; i < 8; ++i)
    {
        float x = std::fmod(i * 173.0f, b.getWidth());
        g.fillEllipse(x, b.getHeight() - 18 - (i % 3) * 8, 26, 12);
    }
    // seaweed
    g.setColour(juce::Colour(0xFF2E8B57));
    for (int i = 0; i < 3; ++i)
    {
        float x = 30 + i * 40;
        g.fillRect(x, b.getHeight() - 90.0f, 8.0f, 70.0f);
    }
}

void GlubGlubEditor::resized()
{
    auto b = getLocalBounds();
    drawer.setBounds(b.removeFromBottom(drawer.getCurrentHeight()));
    speech.setBounds(b.removeFromTop(84));
    fish.setBounds(b);
    bubbles.setBounds(b);
}

void GlubGlubEditor::timerCallback()
{
    float energy = proc.vibe.energy.load();
    float bright = proc.vibe.brightness.load();
    float pulse = proc.vibe.beatPulse.load();
    float phase = proc.vibe.beatPhase.load();
    int inten = proc.vibe.intensity.load();
    int bar = proc.vibe.barCount.load();
    bool bubblesOn = proc.apvts.getRawParameterValue("bubblesOn")->load() > 0.5f;
    float speechRate = proc.apvts.getRawParameterValue("speechRate")->load();

    fish.setVibe(energy, bright, pulse, phase, inten, bar);
    bubbles.setEnabled(bubblesOn);
    bubbles.update(energy, fish.getMouthPosition());
    double now = juce::Time::getMillisecondCounterHiRes() / 1000.0 - startTime;
    speech.update(now, energy, inten, speechRate, rng);
}
