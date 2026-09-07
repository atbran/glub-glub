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

void HypeMeter::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

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
