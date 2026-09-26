#include "HypeMeter.h"

HypeMeter::HypeMeter()
{
    setInterceptsMouseClicks(false, false);
}

void HypeMeter::setHype(float h)
{
    h = juce::jlimit(0.0f, 1.0f, h);
    hype += (h - hype) * 0.18f;
    repaint();
}

void HypeMeter::setTempo(float newBpm, int newSource, float phase)
{
    bpm = newBpm;
    source = newSource;
    beatPhase = phase;
}

void HypeMeter::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    auto tempoRow = b.removeFromBottom(juce::jmax(0.0f, b.getHeight() - 18.0f));
    b = b.withHeight(18.0f);

    if (tempoRow.getHeight() > 8.0f)
    {
        tempoRow.removeFromTop(3.0f);
        // Beat dot: flashes on the beat, dim between.
        const float flash = beatPhase >= 0.0f ? std::pow(1.0f - beatPhase, 4.0f) : 0.0f;
        auto dot = tempoRow.removeFromLeft(10.0f).withSizeKeepingCentre(8.0f, 8.0f);
        g.setColour(juce::Colour(0xFF261712));
        g.fillRect(dot);
        g.setColour(juce::Colour(0xFFFF5A1F).withAlpha(0.25f + 0.75f * flash));
        g.fillRect(dot.reduced(2.0f));
        tempoRow.removeFromLeft(4.0f);
        juce::String text = source == 1 ? juce::String(juce::roundToInt(bpm)) + " BPM  host"
                          : source == 2 ? juce::String(juce::roundToInt(bpm)) + " BPM  by ear"
                                        : juce::String("listening...");
        g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold)));
        g.setColour(juce::Colour(0xFF261712).withAlpha(0.6f));
        g.drawText(text, tempoRow.translated(1.0f, 1.0f), juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xFFFFF5DF).withAlpha(source == 0 ? 0.55f : 0.95f));
        g.drawText(text, tempoRow, juce::Justification::centredLeft);
    }

    g.setColour(juce::Colour(0xFFFFF5DF));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::bold));
    auto label = b.removeFromLeft(36.0f);
    g.drawText("HYPE", label, juce::Justification::centredLeft);

    auto bar = b;
    g.setColour(juce::Colour(0xFF261712));
    g.fillRect(bar);
    bar.reduce(2.0f, 2.0f);
    g.setColour(juce::Colour(0xFF1E3346));
    g.fillRect(bar);

    const int segs = 10;
    const float gap = 2.0f;
    float segW = (bar.getWidth() - (segs - 1) * gap) / (float) segs;

    for (int i = 0; i < segs; ++i)
    {
        float fill = juce::jlimit(0.0f, 1.0f, hype * (float) segs - (float) i);
        if (fill <= 0.0f) break;
        juce::Colour c = i < 4 ? juce::Colour(0xFF3FA7C4)
                       : i < 7 ? juce::Colour(0xFFFFC93C)
                               : juce::Colour(0xFFFF5A1F);
        g.setColour(c);
        g.fillRect(bar.getX() + (float) i * (segW + gap), bar.getY(), segW * fill, bar.getHeight());
    }
}
