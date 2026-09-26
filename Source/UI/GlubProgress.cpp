#include "GlubProgress.h"
#include <cmath>

namespace
{
    juce::File& storageOverride()
    {
        static juce::File file;
        return file;
    }
}

const GlubProgress::Unlock GlubProgress::unlocks[GlubProgress::numUnlocks] = {
    { "party hat",      2,  Unlock::Hat,   1 },
    { "goldfish skin",  3,  Unlock::Skin,  1 },
    { "gold chain",     4,  Unlock::Chain, 1 },
    { "shubunkin skin", 5,  Unlock::Skin,  2 },
    { "crown",          6,  Unlock::Hat,   2 },
    { "neon skin",      8,  Unlock::Skin,  3 },
    { "golden skin",    10, Unlock::Skin,  4 },
};

void GlubProgress::overrideStorageFile(const juce::File& file) { storageOverride() = file; }

GlubProgress::GlubProgress()
{
    if (storageOverride() != juce::File())
    {
        props = std::make_unique<juce::PropertiesFile>(storageOverride(), juce::PropertiesFile::Options());
    }
    else
    {
        juce::PropertiesFile::Options options;
        options.applicationName = "Glub-Glub";
        options.filenameSuffix = "settings";
        options.folderName = "GlubGlub";
        options.osxLibrarySubFolder = "Application Support";
        props = std::make_unique<juce::PropertiesFile>(options);
    }
    xp = (float) props->getDoubleValue("xp", 0.0);
    while (xp >= xpToReach(level + 1)) ++level;
    // Wardrobe choices are re-checked against the level in case the file was edited.
    setSkin(props->getIntValue("skin", 0));
    setHat(props->getIntValue("hat", 0));
    setChain(props->getBoolValue("chain", false));
}

GlubProgress::~GlubProgress() { save(); }

float GlubProgress::xpToReach(int targetLevel)
{
    float total = 0.0f;
    for (int n = 1; n < targetLevel; ++n)
        total += 60.0f * std::pow((float) n, 1.3f);
    return total;
}

void GlubProgress::addXp(float amount)
{
    if (amount <= 0.0f) return;
    xp += amount;
    unsavedXp += amount;
    bool leveled = false;
    while (xp >= xpToReach(level + 1)) { ++level; ++levelUpSerial; leveled = true; }
    if (leveled || unsavedXp > 30.0f) save();
}

int GlubProgress::levelRequiredForSkin(int s)
{
    for (const auto& u : unlocks)
        if (u.kind == Unlock::Skin && u.id == s) return u.level;
    return 1;
}

int GlubProgress::levelRequiredForHat(int h)
{
    for (const auto& u : unlocks)
        if (u.kind == Unlock::Hat && u.id == h) return u.level;
    return 1;
}

int GlubProgress::levelRequiredForChain()
{
    for (const auto& u : unlocks)
        if (u.kind == Unlock::Chain) return u.level;
    return 1;
}

juce::String GlubProgress::unlockedAt(int lvl)
{
    juce::StringArray names;
    for (const auto& u : unlocks)
        if (u.level == lvl) names.add(u.name);
    return names.joinIntoString(", ");
}

void GlubProgress::setSkin(int s)
{
    if (s >= 0 && s < 5 && level >= levelRequiredForSkin(s)) skin = s;
}

void GlubProgress::setHat(int h)
{
    if (h >= 0 && h <= 2 && level >= levelRequiredForHat(h)) hat = h;
}

void GlubProgress::setChain(bool on)
{
    chain = on && level >= levelRequiredForChain();
}

void GlubProgress::save()
{
    props->setValue("xp", (double) xp);
    props->setValue("skin", skin);
    props->setValue("hat", hat);
    props->setValue("chain", chain);
    props->saveIfNeeded();
    unsavedXp = 0.0f;
}
