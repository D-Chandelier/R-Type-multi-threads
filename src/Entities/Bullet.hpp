#pragma once
#include "../Core/Config.hpp"
#include <SFML/Graphics.hpp>

enum class BulletType
{
    PLAYER,
    ENEMY
};

constexpr float BULLET_HEIGHT = 5.f;
constexpr float BULLET_WIDTH = 10.f;

struct Bullet
{
    uint32_t id;
    sf::Vector2f position;
    sf::Vector2f velocity;
    uint8_t ownerId;
    bool active = true;

    void update(float dt);
};
