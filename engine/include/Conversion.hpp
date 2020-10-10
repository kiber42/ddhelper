#pragma once

class Hero;
enum class HeroClass;
enum class HeroRace;

#include <functional>

class Conversion
{
public:
    Conversion(HeroClass theClass, HeroRace race);
    int getPoints() const;
    int getThreshold() const;

    // Add points, returns true if threshold was reached
    [[nodiscard]] bool addPoints(int pointsAdded);

    void applyBonus(Hero& hero);

private:
    int points;
    int threshold;
    std::function<void(Hero&)> bonus;
};
