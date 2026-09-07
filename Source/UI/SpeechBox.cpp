#include "SpeechBox.h"

SpeechBox::SpeechBox() {}

juce::String SpeechBox::pick(int intensity, float energy, juce::Random& rng)
{
    static const char* idle[] = { "glub glub!", "blub... blub!", "i love you!", "is that bass? hi bass!", "bubble break!", "wheee~" };
    static const char* low[] = { "so sleepy... so glubby...", "lofi glub ~", "drifting... blub", "cozy water today" };
    static const char* med[] = { "oh! i feel that groove!", "glub glub! funky!", "wiggle wiggle!", "this beat tickles my fins!" };
    static const char* high[] = { "WHOA!!! DROP!! GLUB!!", "HYPE HYPE BUBBLES!!", "SPINNING!! WHEEE!!", "TOO MUCH FUN!! BLUB!!" };
    auto choose = [&](const char** arr, int n) { return juce::String(arr[(int) (rng.nextFloat() * n) % n]); };
    if (intensity == 2) return choose(high, 4);
    if (intensity == 1) return choose(med, 4);
    if (energy < 0.12f) return choose(low, 4);
    return choose(idle, 6);
}

void SpeechBox::update(double nowSec, float energy, int intensity, float speechRateSec, juce::Random& rng)
{
    if (nowSec >= nextAt)
    {
        current = pick(intensity, energy, rng);
        lastChange = nowSec;
        shownAt = nowSec;
        // at most once per 30s enforced by min clamp on rate
        double rate = juce::jlimit(30.0, 90.0, (double) speechRateSec);
        nextAt = nowSec + rate * (0.7 + rng.nextFloat() * 0.6);
    }
    float target = (nowSec - shownAt < 4.0 && current.isNotEmpty()) ? 1.0f : 0.0f;
    alpha += (target - alpha) * 0.12f;
    if (alpha > 0.02f) repaint();
}

void SpeechBox::paint(juce::Graphics& g)
{
    if (alpha <= 0.02f || current.isEmpty()) return;
    auto b = getLocalBounds().toFloat();
    juce::Rectangle<float> box(b.getWidth() * 0.08f, 12, b.getWidth() * 0.84f, 56);
    g.setOpacity(alpha);
    g.setColour(juce::Colour(0xFFFFF8EC));
    g.fillRect(box);
    g.setColour(juce::Colour(0xFF1A1A1A));
    g.drawRect(box, 3.0f);
    g.setColour(juce::Colour(0xFF333333));
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText(current, box.reduced(8), juce::Justification::centred, true);
    // little tail
    juce::Path tail;
    tail.addTriangle(box.getX() + 40, box.getBottom(), box.getX() + 58, box.getBottom(), box.getX() + 46, box.getBottom() + 12);
    g.setColour(juce::Colour(0xFFFFF8EC));
    g.fillPath(tail);
    g.setOpacity(1.0f);
}
