#pragma once
#include <SFML/System/Vector2.hpp>

enum class BonusType
{
    RocketX3,
    HealthX1,
    Shield,
    FireRateUp,
    ScoreBoost
};

struct Bonus
{
    uint32_t id;
    BonusType type;
    sf::Vector2f position;
    bool active = true;
};

struct BonusStats
{
    int turretsDestroyed = 0;
};
