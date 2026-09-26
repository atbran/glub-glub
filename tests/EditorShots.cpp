// Renders the real editor offscreen while the real processor hears a synthetic
// groove, so UI changes can be reviewed (and timed) without a DAW or a screen.
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include <iostream>

struct EditorShots
{
    GlubGlubProcessor proc;
    std::unique_ptr<GlubGlubEditor> editor;
    juce::File out;
    juce::Random rng { 5 };
    long long sample = 0;
    const double sr = 48000.0;

    float groove(double bpm, float gain)
    {
        const double t = sample++ / sr;
        const double beatLen = 60.0 / bpm;
        const double beats = t / beatLen;
        const double inBeat = (beats - std::floor(beats)) * beatLen;
        const double pitch = 50.0 + 90.0 * std::exp(-inBeat / 0.03);
        float v = 0.85f * (float) (std::exp(-inBeat / 0.09) * std::sin(juce::MathConstants<double>::twoPi * pitch * inBeat));
        const float noise = rng.nextFloat() * 2.0f - 1.0f;
        if (((int) beats) % 2 == 1) v += 0.35f * noise * (float) std::exp(-inBeat / 0.07);
        if (inBeat >= beatLen * 0.5) v += 0.2f * noise * (float) std::exp(-(inBeat - beatLen * 0.5) / 0.015);
        v += 0.12f * (float) std::sin(juce::MathConstants<double>::twoPi * 220.0 * t);
        return v * gain;
    }

    // Advance audio and UI together in real time (the editor reads the wall clock).
    double run(double seconds, double bpm, float gain, std::function<void(int, const juce::Image&)> onFrame = {})
    {
        juce::AudioBuffer<float> buffer(2, 800);
        juce::MidiBuffer midi;
        double paintMs = 0.0;
        const int frames = (int) (seconds * 60.0);
        for (int f = 0; f < frames; ++f)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                const float v = groove(bpm, gain);
                buffer.setSample(0, i, v);
                buffer.setSample(1, i, v);
            }
            proc.processBlock(buffer, midi);
            const double start = juce::Time::getMillisecondCounterHiRes();
            editor->timerCallback();
            auto frame = editor->createComponentSnapshot(editor->getLocalBounds()); // paint updates the koi's mouth
            if (onFrame) onFrame(f, frame);
            paintMs += juce::Time::getMillisecondCounterHiRes() - start;
            juce::Thread::sleep(juce::jmax(1, 16 - (int) (juce::Time::getMillisecondCounterHiRes() - start)));
        }
        return paintMs / frames;
    }

    void shot(const juce::String& name)
    {
        auto image = editor->createComponentSnapshot(editor->getLocalBounds());
        juce::FileOutputStream stream(out.getChildFile(name + ".png"));
        stream.setPosition(0);
        stream.truncate();
        juce::PNGImageFormat().writeImageToStream(image, stream);
        std::cout << "wrote " << name << ".png  tempo source " << proc.vibe.tempoSource.load()
                  << " bpm " << proc.vibe.bpm.load() << std::endl;
    }

    void setParam(const char* id, float value)
    {
        auto* p = proc.apvts.getParameter(id);
        p->setValueNotifyingHost(p->convertTo0to1(value));
    }

    void dropFood(int count)
    {
        auto& shaker = editor->shaker;
        for (int i = 0; i < count; ++i)
        {
            FoodShaker::Pellet p;
            p.pos = { 120.0f + 30.0f * i, 60.0f + 8.0f * (i % 3) };
            p.vel = { 0.0f, 10.0f };
            p.bornAt = shaker.lastFrameTime;
            p.shape = i % 3;
            shaker.pellets.push_back(p);
        }
    }

    int runAll()
    {
        out = juce::File::getCurrentWorkingDirectory().getChildFile("build-msvc/editor-shots");
        out.createDirectory();
        proc.setPlayConfigDetails(2, 2, sr, 800);
        proc.prepareToPlay(sr, 800);
        editor.reset(static_cast<GlubGlubEditor*>(proc.createEditor()));
        editor->setSize(500, 500);

        run(9.0, 124.0, 1.0f);
        shot("lagoon-party");

        setParam("theme", 1);
        run(3.0, 124.0, 0.35f);
        shot("midnight");

        setParam("theme", 2);
        dropFood(5);
        run(0.8, 124.0, 0.2f);
        shot("sunset-feeding-a");
        run(2.2, 124.0, 0.2f);
        shot("sunset-feeding-b");
        run(3.0, 124.0, 0.2f);
        std::cout << "pellets left after 6s: " << editor->shaker.pellets.size()
                  << "  fullness " << editor->fish.getFullness() << std::endl;
        shot("sunset-fed");

        // Hero: a loud groove until hype pegs and the disco ball drops in,
        // then GIF frames at 15 fps.
        setParam("theme", 0);
        setParam("glassesOn", 1);
        run(12.0, 128.0, 1.4f);
        shot("hero");
        auto gifDir = out.getChildFile("gif");
        gifDir.deleteRecursively();
        gifDir.createDirectory();
        run(8.0, 128.0, 1.4f, [&](int f, const juce::Image& image)
        {
            if (f % 4 != 0) return;
            juce::FileOutputStream frameStream(gifDir.getChildFile(juce::String(f / 4).paddedLeft('0', 3) + ".png"));
            juce::PNGImageFormat().writeImageToStream(image, frameStream);
        });
        std::cout << "disco showing: " << editor->disco.isShowing() << std::endl;
        setParam("glassesOn", 0);

        editor->panelWanted = true;
        run(1.0, 124.0, 0.5f);
        shot("panel-open");
        editor->panelWanted = false;

        editor->setSize(1024, 1024);
        const double ms = run(3.0, 124.0, 1.0f);
        shot("lagoon-1024");
        std::cout << "average tick + full repaint at 1024x1024: " << ms << " ms" << std::endl;
        editor = nullptr;
        return 0;
    }
};

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    EditorShots shots;
    return shots.runAll();
}
