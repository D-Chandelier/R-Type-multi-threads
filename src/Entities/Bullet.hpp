#pragma once
#include "../Core/Config.hpp"
#include <SFML/Graphics.hpp>

enum class BulletOwner
{
    PLAYER,
    ENEMY
};

enum class BulletType
{
    LINEAR,
    HOMING_MISSILE
};

constexpr float BULLET_HEIGHT = 5.f;
constexpr float BULLET_WIDTH = 10.f;

constexpr float MISSILE_START_SPEED = 120.f;
constexpr float MISSILE_MAX_SPEED = 700.f;
constexpr float MISSILE_ACCELERATION = 900.f;
constexpr float MISSILE_TURN_RATE = 4.5f; // radians/sec
constexpr float MISSILE_LAUNCH_TIME = 0.2f;
constexpr float MISSILE_RANGE = 900.f;

struct Bullet
{
    uint32_t id;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float damage;
    uint8_t ownerId;
    BulletOwner owner = BulletOwner::PLAYER;
    bool active = true;

    // Missile only
    BulletType type = BulletType::LINEAR;
    float lifetime = 0.f;
    uint32_t targetId = 0;

    void update(float dt);
};
