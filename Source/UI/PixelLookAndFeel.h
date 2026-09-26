#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Chunky pixel-art controls so the settings panel matches the koi.
class PixelLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static constexpr juce::uint32 ink = 0xFF261712, cream = 0xFFFDF6E3, koi = 0xFFE8491D,
                                  koiLight = 0xFFFF8148, slate = 0xFF1E3346, slateLight = 0xFF2E4A63;

    PixelLookAndFeel()
    {
        setColour(juce::Label::textColourId, juce::Colour(cream));
        setColour(juce::ToggleButton::textColourId, juce::Colour(cream));
        setColour(juce::TextButton::textColourOffId, juce::Colour(cream));
        setColour(juce::TextButton::textColourOnId, juce::Colour(ink));
        setColour(juce::TooltipWindow::backgroundColourId, juce::Colour(cream));
        setColour(juce::TooltipWindow::textColourId, juce::Colour(ink));
        setColour(juce::TooltipWindow::outlineColourId, juce::Colour(ink));
    }

    static juce::Font pixelFont(float size = 13.0f)
    {
        return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::bold));
    }

    juce::Font getLabelFont(juce::Label&) override { return pixelFont(12.0f); }
    juce::Font getTextButtonFont(juce::TextButton&, int h) override { return pixelFont(juce::jmin(13.0f, h * 0.55f)); }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float, float,
                          juce::Slider::SliderStyle, juce::Slider&) override
    {
        const int trackH = 6;
        const int ty = y + h / 2 - trackH / 2;
        g.setColour(juce::Colour(ink));
        g.fillRect(x, ty - 2, w, trackH + 4);
        g.setColour(juce::Colour(slate));
        g.fillRect(x + 2, ty, w - 4, trackH);
        // Filled portion in chunky 4px steps.
        const int filled = ((int) pos - x) / 4 * 4;
        g.setColour(juce::Colour(koi));
        g.fillRect(x + 2, ty, juce::jmax(0, filled - 2), trackH);
        g.setColour(juce::Colour(koiLight));
        g.fillRect(x + 2, ty, juce::jmax(0, filled - 2), 2);

        const int thumbW = 10, thumbH = 16;
        const int tx = juce::jlimit(x, x + w - thumbW, (int) pos - thumbW / 2);
        const int thy = y + h / 2 - thumbH / 2;
        g.setColour(juce::Colour(ink));
        g.fillRect(tx, thy, thumbW, thumbH);
        g.setColour(juce::Colour(cream));
        g.fillRect(tx + 2, thy + 2, thumbW - 4, thumbH - 4);
        g.setColour(juce::Colour(0xFFE3D2A8));
        g.fillRect(tx + 2, thy + thumbH - 5, thumbW - 4, 3);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool) override
    {
        const int box = 12;
        const int by = b.getHeight() / 2 - box / 2;
        g.setColour(juce::Colour(ink));
        g.fillRect(2, by, box, box);
        g.setColour(b.getToggleState() ? juce::Colour(koi) : juce::Colour(highlighted ? slateLight : slate));
        g.fillRect(4, by + 2, box - 4, box - 4);
        if (b.getToggleState())
        {
            g.setColour(juce::Colour(cream));
            g.fillRect(5, by + 6, 2, 2);
            g.fillRect(7, by + 7, 2, 2);
            g.fillRect(9, by + 4, 2, 3);
        }
        g.setColour(b.findColour(juce::ToggleButton::textColourId).withAlpha(b.isEnabled() ? 1.0f : 0.5f));
        g.setFont(pixelFont(12.0f));
        g.drawText(b.getButtonText(), box + 7, 0, b.getWidth() - box - 7, b.getHeight(), juce::Justification::centredLeft);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool highlighted, bool down) override
    {
        auto r = b.getLocalBounds();
        const bool on = b.getToggleState();
        g.setColour(juce::Colour(ink));
        g.fillRect(r);
        r.reduce(2, 2);
        auto face = on ? juce::Colour(0xFFFFC93C) : juce::Colour(highlighted ? slateLight : slate);
        if (down) face = face.darker(0.2f);
        g.setColour(face);
        g.fillRect(r);
        if (!down)
        {
            g.setColour(face.brighter(0.25f));
            g.fillRect(r.removeFromTop(2));
            g.setColour(face.darker(0.35f));
            g.fillRect(b.getLocalBounds().reduced(2).removeFromBottom(2));
        }
    }

    void drawTooltip(juce::Graphics& g, const juce::String& text, int w, int h) override
    {
        g.fillAll(juce::Colour(cream));
        g.setColour(juce::Colour(ink));
        g.drawRect(0, 0, w, h, 2);
        g.setFont(pixelFont(12.0f));
        g.drawFittedText(text, 6, 2, w - 12, h - 4, juce::Justification::centredLeft, 3);
    }
};
