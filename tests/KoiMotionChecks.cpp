#include "../Source/UI/KoiFish.h"
#include "../Source/UI/HypeEnvelope.h"
#include <iostream>

struct KoiMotionChecks
{
    static int run()
    {
        KoiFish fish;
        fish.setSize(500, 388);
        fish.activeMove = KoiFish::MoveType::Roll;
        fish.moveT = 0.2f;
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
                if (fish.spriteImg.getPixelAt(x - (int) fish.spriteOrigin.x, y - (int) fish.spriteOrigin.y).getAlpha() > 128)
                { if (first < 0) first = x; last = x; }
            for (int x = first + 1; first >= 0 && x < last; ++x)
                if (fish.spriteImg.getPixelAt(x - (int) fish.spriteOrigin.x, y - (int) fish.spriteOrigin.y).getAlpha() < 16) ++holes;
        }
        std::cout << "Roll body transparent interior pixels: " << holes << '\n';
        fish.moveT = 0.4f;
        fish.triggerMove(KoiFish::MoveType::Roll);
        const bool continuous = fish.doing(KoiFish::MoveType::Roll) && fish.moveT == 0.4f;
        std::cout << "Repeated move preserves active progress: " << continuous << '\n';
        bool passed = holes == 0 && continuous;
        auto check = [&](bool ok, const char* label)
        {
            std::cout << label << ": " << (ok ? "PASS" : "FAIL") << '\n';
            passed &= ok;
        };
        fish.triggerMove(KoiFish::MoveType::Twerk);
        check(fish.doing(KoiFish::MoveType::Roll) && fish.queuedMove == KoiFish::MoveType::Twerk, "Different move queues without interrupting roll");
        for (int i = 0; i < 90; ++i) fish.setVibe(0, 0, 0, 0, 0, 1, 0);
        check(fish.activeMove == KoiFish::MoveType::Twerk, "Queued twerk starts after roll lands");

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
        check(!quantised.doing(KoiFish::MoveType::Roll) && quantised.queuedMove == KoiFish::MoveType::Roll, "Manual dance waits for a downbeat");
        quantised.setVibe(0, 0, 0, 0.99f, 0, 1, 0);
        check(quantised.doing(KoiFish::MoveType::Roll), "Queued dance enters on the beat boundary");
        for (const float tempo : { 90.0f, 128.0f, 174.0f })
        {
            KoiFish timed;
            timed.triggerMove(KoiFish::MoveType::Roll);
            const int ticks = (int) std::round(2.0f * 60.0f / tempo * 120.0f);
            for (int i = 0; i < ticks; ++i)
                timed.setVibe(0, 0, 0, std::fmod((i + 1) * tempo / 7200.0f, 1.0f), 0, 1, 0, 0, 1.0f / 120, tempo);
            check(std::abs(timed.moveT - 0.5f) < 0.01f, "Roll is halfway after two beats at different tempos");
        }
        KoiFish twerker;
        twerker.setSize(500, 388);
        twerker.activeMove = KoiFish::MoveType::Twerk;
        twerker.moveT = 0.5f;
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

        KoiFish breaker;
        breaker.setSize(500, 388);
        breaker.triggerMove(KoiFish::MoveType::Breakdance);
        check(breaker.doing(KoiFish::MoveType::Breakdance), "Breakdance triggers cleanly");
        for (int i = 0; i < 60; ++i) breaker.setVibe(0.8f, 0.5f, 0.95f, std::fmod(i * 128.0f / 3600.0f, 1.0f), 2, 1, 0, 0.9f, 1.0f / 60.0f, 128.0f);
        check(breaker.doing(KoiFish::MoveType::Breakdance) && breaker.moveT > 0.4f, "Breakdance advances through headspin phase");

        // Isolate phase boundaries from the host clock and raster quantisation.
        for (const bool right : { false, true })
        {
            KoiFish dancer;
            dancer.setSize(500, 388);
            dancer.facingRight = right;
            dancer.activeMove = KoiFish::MoveType::Breakdance;
            auto renderAt = [&](float progress)
            {
                dancer.moveT = progress;
                juce::Image image(juce::Image::ARGB, 500, 388, true, juce::SoftwareImageType());
                juce::Graphics g(image);
                dancer.paint(g);
                return image;
            };
            for (float boundary : { 0.20f, 0.22f, 0.62f, 0.68f, 0.72f, 0.78f, 0.92f, 1.0f })
            {
                const auto before = renderAt(boundary - 0.00001f);
                const auto after = renderAt(boundary + 0.00001f);
                int changed = 0;
                for (int y = 0; y < 388; ++y)
                    for (int x = 0; x < 500; ++x)
                        if (std::abs((int) before.getPixelAt(x, y).getAlpha()
                                   - (int) after.getPixelAt(x, y).getAlpha()) > 64) ++changed;
                std::cout << "Breakdance boundary " << boundary << " changed pixels: " << changed << '\n';
                check(changed < 150, "Breakdance silhouette blends across phase boundaries");
            }
            renderAt(0.3f);
            const auto planted = dancer.getMouthPosition();
            float maxDrift = 0;
            for (int sample = 0; sample <= 40; ++sample)
            {
                dancer.danceBeat = sample / 40.0f;
                dancer.energy = 0.8f;
                dancer.swimPhase = sample * 0.2f;
                renderAt(0.3f + sample * 0.01f);
                maxDrift = juce::jmax(maxDrift, planted.getDistanceFrom(dancer.getMouthPosition()));
            }
            std::cout << "Headspin contact drift: " << maxDrift << '\n';
            check(maxDrift < 1.0f, "Headspin keeps contact planted through beats and tail motion");
        }

        // Feeding: the mouth must reach a pellet, dancing waits, and bites fill him up.
        for (const auto pelletPos : { juce::Point<float>(380, 120), juce::Point<float>(90, 300) })
        {
            KoiFish diner;
            diner.setSize(500, 388);
            diner.paint(graphics);
            diner.setFoodTarget(pelletPos);
            float closest = 1.0e6f;
            bool onlyTurns = true;
            for (int i = 0; i < 180; ++i)
            {
                diner.setVibe(0.7f, 0.5f, 0.3f, -1, 2, i / 30, 1.0f, 0.9f);
                onlyTurns &= diner.getActiveMove() == KoiFish::MoveType::None || diner.getActiveMove() == KoiFish::MoveType::Flip;
                diner.paint(graphics);
                closest = juce::jmin(closest, diner.getMouthPosition().getDistanceFrom(pelletPos));
            }
            check(onlyTurns, "Only turning around happens while chasing food");
            std::cout << "Closest mouth approach to pellet: " << closest << std::endl;
            check(closest < 14.0f, "Koi swims its mouth to the pellet within three seconds");
            diner.chomp();
            check(diner.getFullness() > 0.0f && diner.chompT == 1.0f, "Chomp fills the belly and opens the mouth");
            diner.setFoodTarget(std::nullopt);
            for (int i = 0; i < 240; ++i) { diner.setVibe(0, 0, 0, -1, 0, 1, 0); diner.paint(graphics); }
            check(diner.roam.getDistanceFromOrigin() < 12.0f, "Koi drifts home after dinner");
        }

        // At full energy the tail fan must stay a solid fin, not an outline-only lattice.
        {
            KoiFish thrasher;
            thrasher.setSize(500, 388);
            float worstSolid = 1.0f;
            for (int i = 0; i < 240; ++i)
            {
                thrasher.setVibe(1.0f, 0.8f, 1.0f, std::fmod(i * 128.0f / 3600.0f, 1.0f), 2, i / 120, 0, 1.0f, 1.0f / 60, 128);
                std::vector<char> cells;
                thrasher.buildGrid(cells);
                int fin = 0, outline = 0;
                for (int y = 0; y < KoiFish::GRID_H; ++y)
                    for (int x = 0; x < 9; ++x)
                    {
                        const char c = cells[(size_t) (y * KoiFish::GRID_W + x)];
                        if (c == 'O') ++outline;
                        else if (c == 'f' || c == '5' || c == '6') ++fin;
                    }
                const float solid = fin / (float) juce::jmax(1, fin + outline);
                if (solid < worstSolid)
                {
                    worstSolid = solid;
                    std::cout << "Tail grid at frame " << i << " (solid " << solid << "):\n";
                    for (int y = 0; y < KoiFish::GRID_H; ++y)
                        std::cout << std::string(cells.begin() + y * KoiFish::GRID_W, cells.begin() + (y + 1) * KoiFish::GRID_W) << '\n';
                }
            }
            check(worstSolid > 0.3f, "Tail fan stays solid at full energy");
        }

        const auto output = juce::File::getCurrentWorkingDirectory().getChildFile("build-msvc/motion-review");
        output.createDirectory();
        const char* names[] = { "Worm", "Barrel Roll", "Spin", "Flip", "Shuffle", "Head Bop", "Tail Shimmy", "Figure Eight", "Twerk", "Breakdance" };
        juce::Image sheet(juce::Image::RGB, 1000, 10 * 180, true, juce::SoftwareImageType());
        juce::Graphics sg(sheet);
        sg.fillAll(juce::Colour(0xff183448));
        for (int move = 1; move <= 10; ++move)
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
                if (move == 2 || move == 8 || move == 9 || move == 10)
                {
                    auto file = output.getChildFile(juce::String(move == 2 ? "roll-" : move == 8 ? "eight-" : move == 9 ? "twerk-" : "breakdance-") + juce::String(frameIndex).paddedLeft('0', 3) + ".png");
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
