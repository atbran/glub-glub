#include "../Source/UI/KoiFish.h"
#include "../Source/UI/HypeEnvelope.h"
#include <iostream>

struct KoiMotionChecks
{
    static int run()
    {
        KoiFish fish;
        fish.setSize(500, 388);
        fish.rollActive = true;
        fish.rollT = 0.2f;
        juce::Image frame(juce::Image::ARGB, 500, 388, true, juce::SoftwareImageType());
        juce::Graphics graphics(frame);
        fish.paint(graphics);
        // A connected body row must stay connected when stretched by a roll.
        // Ignore the tail region, whose silhouette intentionally has two lobes.
        int holes = 0;
        for (int y = 180; y < 210; ++y)
        {
            int first = -1, last = -1;
            for (int x = 255; x < 350; ++x)
                if (fish.spriteImg.getPixelAt(x, y).getAlpha() > 128)
                { if (first < 0) first = x; last = x; }
            for (int x = first + 1; first >= 0 && x < last; ++x)
                if (fish.spriteImg.getPixelAt(x, y).getAlpha() < 16) ++holes;
        }
        std::cout << "Roll body transparent interior pixels: " << holes << '\n';
        fish.rollT = 0.4f;
        fish.triggerMove(KoiFish::MoveType::Roll);
        const bool continuous = fish.rollActive && fish.rollT == 0.4f;
        std::cout << "Repeated move preserves active progress: " << continuous << '\n';
        bool passed = holes == 0 && continuous;
        auto check = [&](bool ok, const char* label)
        {
            std::cout << label << ": " << (ok ? "PASS" : "FAIL") << '\n';
            passed &= ok;
        };
        fish.triggerMove(KoiFish::MoveType::Twerk);
        check(fish.rollActive && fish.queuedMove == KoiFish::MoveType::Twerk, "Different move queues without interrupting roll");
        for (int i = 0; i < 90; ++i) fish.setVibe(0, 0, 0, 0, 0, 1, 0);
        check(fish.extraMove == KoiFish::MoveType::Twerk, "Queued twerk starts after roll lands");

        KoiFish thirty, sixty;
        for (int i = 0; i < 60; ++i) thirty.setVibe(0, 0, 0, -1, 0, 1, 0, 0, 1.0f / 30.0f);
        for (int i = 0; i < 120; ++i) sixty.setVibe(0, 0, 0, -1, 0, 1, 0, 0, 1.0f / 60.0f);
        check(std::abs(thirty.swimPhase - sixty.swimPhase) < 0.001, "Swimming speed independent of update rate");
        sixty.setGlassesOn(true);
        for (int i = 0; i < 60; ++i) sixty.setVibe(0, 0, 0, -1, 0, 1, 0);
        check(sixty.shadesT == 1.0f, "Glasses stay on during silence");
        sixty.setGlassesOn(false);
        for (int i = 0; i < 60; ++i) sixty.setVibe(1, 0, 1, 0, 2, 1, 1, 1);
        check(sixty.shadesT == 0.0f, "Glasses stay off even during feeding and max hype");

        HypeEnvelope ordinary, loud, transient, food, drop;
        float ordinaryLevel = 0, loudLevel = 0, transientLevel = 0, foodLevel = 0, dropLevel = 0;
        for (int i = 0; i < 600; ++i)
        {
            ordinaryLevel = ordinary.update(0.5f, i % 30 == 0 ? 1.0f : 0.0f, 0, 1.0f / 60);
            loudLevel = loud.update(0.95f, 0.8f, 0, 1.0f / 60);
            foodLevel = food.update(0, 0, 1, 1.0f / 60);
        }
        for (int i = 0; i < 6; ++i) transientLevel = transient.update(1, 1, 0, 1.0f / 60);
        for (int i = 0; i < 120; ++i) drop.update(0.2f, 0.1f, 0, 1.0f / 60);
        for (int i = 0; i < 60; ++i) dropLevel = drop.update(0.95f, 0.8f, 0, 1.0f / 60);
        check(ordinaryLevel < 0.5f, "Moderate music does not max hype");
        check(loudLevel > 0.97f, "Sustained loud music reaches max hype");
        check(transientLevel < 0.3f, "Isolated peak cannot max hype");
        check(foodLevel <= 0.73f, "Food alone cannot max hype");
        check(dropLevel > 0.90f, "Loud drop earns fast hype buildup");
        for (int i = 0; i < 300; ++i) loudLevel = loud.update(0, 0, 0, 1.0f / 60);
        check(loudLevel < 0.05f, "Hype cools down after music stops");

        KoiFish bounce;
        bounce.setSize(500, 388);
        bounce.energy = 0.8f;
        bounce.beatPhase = 0;
        bounce.danceBeat = 0;
        bounce.paint(graphics);
        const float downbeatY = bounce.fishCy;
        bounce.danceBeat = 0.5f;
        bounce.paint(graphics);
        check(downbeatY - bounce.fishCy > 45.0f, "Loud beat bounce has a pronounced impact and lift");
        const auto rightLobe = KoiFish::figureEightPath(0.25f, 500, 388);
        const auto crossing = KoiFish::figureEightPath(0.5f, 500, 388);
        const auto leftLobe = KoiFish::figureEightPath(0.75f, 500, 388);
        check(rightLobe.x > 100 && leftLobe.x < -100 && crossing.getDistanceFromOrigin() < 0.001f,
              "Figure eight has full opposite lobes and a central crossing");
        KoiFish quantised;
        quantised.setVibe(0, 0, 0, 0.35f, 0, 1, 0);
        quantised.triggerMove(KoiFish::MoveType::Roll);
        check(!quantised.rollActive && quantised.queuedMove == KoiFish::MoveType::Roll, "Manual dance waits for a downbeat");
        quantised.setVibe(0, 0, 0, 0.99f, 0, 1, 0);
        check(quantised.rollActive, "Queued dance enters on the beat boundary");
        for (const float tempo : { 90.0f, 128.0f, 174.0f })
        {
            KoiFish timed;
            timed.triggerMove(KoiFish::MoveType::Roll);
            const int ticks = (int) std::round(2.0f * 60.0f / tempo * 120.0f);
            for (int i = 0; i < ticks; ++i)
                timed.setVibe(0, 0, 0, std::fmod((i + 1) * tempo / 7200.0f, 1.0f), 0, 1, 0, 0, 1.0f / 120, tempo);
            check(std::abs(timed.rollT - 0.5f) < 0.01f, "Roll is halfway after two beats at different tempos");
        }
        KoiFish twerker;
        twerker.setSize(500, 388);
        twerker.extraMove = KoiFish::MoveType::Twerk;
        twerker.extraT = 0.5f;
        auto rearHeight = [&]
        {
            std::vector<char> cells;
            twerker.buildGrid(cells);
            float sum = 0, count = 0;
            for (int y = 0; y < KoiFish::GRID_H; ++y)
                for (int x = 0; x < 8; ++x)
                    if (cells[y * KoiFish::GRID_W + x] != '.') { sum += y; ++count; }
            return sum / juce::jmax(1.0f, count);
        };
        twerker.danceBeat = 0;
        const float rearDown = rearHeight();
        twerker.paint(graphics);
        const auto headDown = twerker.mouthPos;
        twerker.danceBeat = 0.5f;
        const float rearUp = rearHeight();
        twerker.paint(graphics);
        check(std::abs(rearDown - rearUp) > 6.0f && headDown.getDistanceFrom(twerker.mouthPos) < 28.0f,
              "Twerk moves the rear strongly while keeping the head relatively planted");

        const auto output = juce::File::getCurrentWorkingDirectory().getChildFile("build-msvc/motion-review");
        output.createDirectory();
        const char* names[] = { "Worm", "Barrel Roll", "Spin", "Flip", "Shuffle", "Head Bop", "Tail Shimmy", "Figure Eight", "Twerk" };
        juce::Image sheet(juce::Image::RGB, 1000, 9 * 180, true, juce::SoftwareImageType());
        juce::Graphics sg(sheet);
        sg.fillAll(juce::Colour(0xff183448));
        for (int move = 1; move <= 9; ++move)
        {
            KoiFish dancer;
            dancer.setSize(500, 388);
            dancer.setGlassesOn(true);
            dancer.shadesT = 1;
            dancer.triggerMove(static_cast<KoiFish::MoveType>(move));
            const bool longMove = move == 8 || move == 9;
            const int frameCount = longMove ? 264 : 132;
            const int snapshotStride = longMove ? 52 : 26;
            for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex)
            {
                // Keep the requested move isolated, with a real 128 BPM beat.
                dancer.setVibe(0.6f, 0.5f, 0.25f, std::fmod(frameIndex / 60.0f * 128.0f / 60.0f, 1.0f), 1, 1, 0, 0.5f, 1.0f / 60, 128);
                juce::Image rendered(juce::Image::RGB, 500, 388, true, juce::SoftwareImageType());
                juce::Graphics rg(rendered);
                rg.fillAll(juce::Colour(0xff183448));
                dancer.paint(rg);
                if (move == 8 && frameIndex % snapshotStride == 0)
                {
                    int redPixels = 0;
                    for (int y = 0; y < 388; y += 2)
                        for (int x = 0; x < 500; x += 2)
                        {
                            const auto c = rendered.getPixelAt(x, y);
                            if (c.getRed() > 140 && c.getRed() > c.getGreen() * 1.4f) ++redPixels;
                        }
                    check(redPixels > 100, "Figure eight keeps the fish opaque while drawing its wake");
                }
                if (frameIndex % snapshotStride == 0 && frameIndex / snapshotStride < 5)
                    sg.drawImage(rendered, juce::Rectangle<float>((frameIndex / snapshotStride) * 200.0f, (move - 1) * 180.0f + 20, 200, 155));
                if (move == 2 || move == 8 || move == 9)
                {
                    auto file = output.getChildFile(juce::String(move == 2 ? "roll-" : move == 8 ? "eight-" : "twerk-") + juce::String(frameIndex).paddedLeft('0', 3) + ".png");
                    juce::FileOutputStream stream(file);
                    stream.setPosition(0); stream.truncate();
                    juce::PNGImageFormat().writeImageToStream(rendered, stream);
                }
            }
            sg.setColour(juce::Colours::white);
            sg.drawText(names[move - 1], 8, (move - 1) * 180, 220, 22, juce::Justification::centredLeft);
            std::cout << "Rendered " << names[move - 1] << std::endl;
        }
        juce::FileOutputStream sheetStream(output.getChildFile("moves.png"));
        sheetStream.setPosition(0); sheetStream.truncate();
        juce::PNGImageFormat().writeImageToStream(sheet, sheetStream);
        return passed ? 0 : 1;
    }
};

int main()
{
    juce::ScopedJuceInitialiser_GUI initialise;
    return KoiMotionChecks::run();
}
