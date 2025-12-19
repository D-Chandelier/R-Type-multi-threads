#pragma once
#include "../Core/Config.hpp"
#include <SFML/Graphics.hpp>

enum class BulletType
{
    PLAYER,
    ENEMY
};

struct Bullet
{
    uint32_t id;
    sf::Vector2f position;
    sf::Vector2f velocity;
    uint8_t ownerId;
    bool active = true;

    void update(float dt)
    {
        position += velocity * dt;

        // Exemple destruction hors écran
        if (position.x < -50 || position.x > Config::Get().windowSize.x + 50 ||
            position.y < -50 || position.y > Config::Get().windowSize.y + 50)
        {
            active = false;
        }
    }
};
