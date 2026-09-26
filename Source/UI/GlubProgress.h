#pragma once
#include <juce_data_structures/juce_data_structures.h>

// Glub's persistent life as a pet: XP, level and wardrobe. It lives in the
// user's app-data folder, not the plugin state, so he is the same fish in
// every project and DAW. All plugin instances in a process share one copy
// (juce::SharedResourcePointer).
class GlubProgress
{
public:
    struct Unlock { const char* name; int level; enum Kind { Skin, Hat, Chain } kind; int id; };
    static constexpr int numUnlocks = 7;
    static const Unlock unlocks[numUnlocks];

    GlubProgress();
    ~GlubProgress();

    void addXp(float amount);
    int getLevel() const { return level; }
    float getXp() const { return xp; }
    float getXpIntoLevel() const { return xp - xpToReach(level); }
    float getXpForThisLevel() const { return xpToReach(level + 1) - xpToReach(level); }
    // Increments on every level-up, so each editor can notice it independently.
    int getLevelUpSerial() const { return levelUpSerial; }

    // Total XP needed to reach a level (level 1 = 0 XP). ~1 min for level 2,
    // ~15 min of dancing for level 5, ~75 min for level 10.
    static float xpToReach(int level);
    static int levelRequiredForSkin(int skin);
    static int levelRequiredForHat(int hat);
    static int levelRequiredForChain();
    static juce::String unlockedAt(int level); // names unlocked by reaching this level

    int getSkin() const { return skin; }
    int getHat() const { return hat; }
    bool getChain() const { return chain; }
    void setSkin(int s);
    void setHat(int h);
    void setChain(bool on);

    void save();

    // Tests point this at a scratch file before the first instance is made.
    static void overrideStorageFile(const juce::File& file);

private:
    std::unique_ptr<juce::PropertiesFile> props;
    float xp = 0.0f, unsavedXp = 0.0f;
    int level = 1, levelUpSerial = 0;
    int skin = 0, hat = 0;
    bool chain = false;
};
