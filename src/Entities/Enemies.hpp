#pragma once
#include <cstdint>
#include <SFML/Graphics.hpp>
#include <mutex>

#include "../Core/Config.hpp"

constexpr float TURRET_HEIGHT = 48.f;
constexpr float TURRET_WIDTH = 48.f;
constexpr float TURRET_FIRE_INTERVAL = 1.0f;

class Client;

enum class EnemyType
{
    TURRET,
    OTHER
};

struct Enemy
{
    uint32_t id;
    EnemyType type;
    sf::Vector2f position{};
    sf::Vector2f velocity;
    float pv = 2;
    bool active = true;
    float points = 10.f;
    float shootCooldown = 0.f; // timer pour tirer
    bool wantsToShoot = false;

    Enemy(sf::Vector2f pos);
    void draw(Client &c, sf::RenderWindow &w);
    void drawTurret(Client &c, sf::RenderWindow &w);

    void update(float dt, float worldX);
    void updateTurret(float dt, float worldX);
};
