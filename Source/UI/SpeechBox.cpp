#include "SpeechBox.h"

SpeechBox::SpeechBox() {}

juce::String SpeechBox::pick(int intensity, float energy, juce::Random& rng)
{
    static const char* idle[] = {
        "Bro did you fall asleep at the fader?",
        "Did your trial of Ableton expire or what?",
        "Zero dBFS, zero bitches.",
        "Is this track paused or is this avant-garde ambient?",
        "My swim bladder is drier than this mix.",
        "Even my filter cutoff is depressed.",
        "Unmute me you coward.",
        "glub... someone hit play before I dry out...",
        "I didn't evolve from primordial soup to sit in silence."
    };
    static const char* low[] = {
        "Lofi beats to disassociate and rot to.",
        "Slap another OTT on it, don't be shy.",
        "This snare sounds like a wet cardboard box.",
        "Needs more sausage fattener, honestly.",
        "Cozy... but where's the drop at, chief?",
        "Bro's mixing at -40 LUFS for ants.",
        "Is the high-pass set to 15kHz or are my gills clogged?",
        "A little more drive, daddy... just a little."
    };
    static const char* med[] = {
        "Okay okay... the bassline kinda thicc though.",
        "Fin-shaking groove! Watch me shake this caudal fin.",
        "That kick is punchy enough to crack my glass.",
        "Ooh yeah, slap that sidechain harder daddy!",
        "Who gave you permission to cook like this?!",
        "My scales are tingling in forbidden ways.",
        "Turn the sub up until the neighbors file a lawsuit!",
        "Mmm yeah, pump that compressor right there.",
        "I'm wiggling so hard my dorsal fin might come off."
    };
    static const char* high[] = {
        "HOLY SHIT THE DROP!! GLUB GLUB GLUB!!",
        "BRICKWALL LIMIT ME STRAIGHT TO HELL!!",
        "CERTIFIED BANGER!! MY GILLS ARE CLIPPING!!",
        "WE'RE IN THE RED, BABY! RED MEANS WARMTH!!",
        "SOMEBODY CALL 911, THIS 808 IS TOO FAT!!",
        "I'M SHAKING MY ASS FIN LIKE IT'S 3 AM IN BERLIN!!",
        "MORE BASS! BLOW OUT MY RETINAS!!",
        "GLUB SO HARD MOTHERFUCKERS WANNA FRY ME!!",
        "SPREAD MY STEREO WIDTH WIDE OPEN!!",
        "MAX HYPE!! GLUB GLUB HYPE EXPLOSION!!"
    };
    auto choose = [&](const char** arr, int n) { return juce::String(arr[(int) (rng.nextFloat() * n) % n]); };
    if (intensity == 2) return choose(high, (int) (sizeof(high) / sizeof(high[0])));
    if (intensity == 1) return choose(med, (int) (sizeof(med) / sizeof(med[0])));
    if (energy < 0.12f) return choose(low, (int) (sizeof(low) / sizeof(low[0])));
    return choose(idle, (int) (sizeof(idle) / sizeof(idle[0])));
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
    juce::Rectangle<float> box(b.getWidth() * 0.08f, b.getHeight() - 68.0f, b.getWidth() * 0.84f, 56.0f);
    g.setOpacity(alpha);
    g.setColour(juce::Colour(0xFFFFF8EC));
    g.fillRect(box);
    g.setColour(juce::Colour(0xFF1A1A1A));
    g.drawRect(box, 3.0f);
    g.setColour(juce::Colour(0xFF333333));
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText(current, box.reduced(8), juce::Justification::centred, true);
    // little tail pointing up at the fish
    juce::Path tail;
    tail.addTriangle(box.getX() + 40, box.getY(), box.getX() + 58, box.getY(), box.getX() + 46, box.getY() - 12.0f);
    g.setColour(juce::Colour(0xFFFFF8EC));
    g.fillPath(tail);
    g.setOpacity(1.0f);
}
