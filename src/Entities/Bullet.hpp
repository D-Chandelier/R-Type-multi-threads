#pragma once
#include <SFML/Graphics.hpp>
#include "../Core/Config.hpp"
#include "Enemies.hpp"

class Client;
class Server;

enum class BulletOwner
{
    PLAYER,
    ENEMY
};

enum class BulletType
{
    LINEAR,
    HOMING_ROCKET
};

constexpr float BULLET_HEIGHT = 5.f;
constexpr float BULLET_WIDTH = 10.f;
constexpr float BULLET_SPEED = 500.f;
constexpr sf::Color BULLET_COLOR = sf::Color::Cyan;

constexpr float ROCKET_HEIGHT = 12.f;
constexpr float ROCKET_WIDTH = 32.f;
constexpr float ROCKET_START_SPEED = 120.f;
constexpr float ROCKET_MAX_SPEED = 700.f;
constexpr float ROCKET_ACCELERATION = 900.f;
constexpr float ROCKET_TURN_RATE = 4.5f; // radians/sec
constexpr float ROCKET_LAUNCH_TIME = 0.3f;
constexpr float ROCKET_RANGE = 900.f;

struct Bullet
{
    uint32_t id;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float damage = 1.f;
    uint8_t ownerId;
    BulletOwner owner = BulletOwner::PLAYER;
    bool active = true;

    // Missile only
    BulletType type = BulletType::LINEAR;
    float lifetime = 0.f;
    uint32_t targetId = 0;

    void update(float dt);
    static Bullet &Get()
    {
        static Bullet instance;
        return instance;
    }
    // Coté CLIENT
    static void updateBulletsClient(Client &client, float dt);
    static void buildBulletQuad(const Bullet &b, float angle, sf::VertexArray &bulletsVA);
    static void buildRocketQuad(const Bullet &b, float angle, sf::VertexArray &rocketsVA);

    static void drawBullets(sf::RenderWindow &w);

    // Coté SERVER
    static void updateBulletsServer(Server &server, float dt);
    static void updateRocketServer(Server &server, Bullet &m, float dt);
    static void spawnTurretBullet(Enemy &t, Server &s);
    ////////////////
    inline static float radToDeg(float r)
    {
        return r * 180.f / 3.14159265f;
    }

    inline static float bulletAngle(const sf::Vector2f &v)
    {
        return radToDeg(std::atan2(v.y, v.x));
    }

    static sf::Vector2f rotatePoint(const sf::Vector2f &p, float angleRad)
    {
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        return {p.x * c - p.y * s, p.x * s + p.y * c};
    }

public:
    sf::VertexArray bulletsVA, rocketsVA;
};
