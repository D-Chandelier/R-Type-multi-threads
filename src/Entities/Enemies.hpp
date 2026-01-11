#pragma once
#include <cstdint>
#include <SFML/Graphics.hpp>
#include <mutex>

#include "../Core/Config.hpp"
#include "../World/Entities.hpp"

constexpr float TURRET_HEIGHT = 48.f;
constexpr float TURRET_WIDTH = 48.f;
constexpr float TURRET_FIRE_INTERVAL = 1.0f;

class Client;
class Server;

enum class EnemyType
{
    DRONE,
    TURRET,
    KAMIKAZE,
    BOSS,
    OTHER
};

enum class EnemySpawnType
{
    Simple,
    Wave,
    Circle,
    Kamikaze,
    Group,
    Boss
};

enum class TurretPlacement
{
    None,
    Surface,
    Bridge,
    Ceiling
};

struct EnemySpawnDesc
{
    float atX = 0.f;

    EnemyArchetypeId archetype;
    EnemySpawnType spawnType;

    int count = 1;
    float spacing = 0.f;

    std::string formation;
    float y = -1.f;
    sf::Vector2f velocity = {0.f, 0.f};
    sf::Vector2f size = {48.f, 48.f};

    bool spawned = false;
};

struct EnemyRuntime
{
    EnemySpawnDesc desc;
    bool spawned = false;
};

struct Enemy
{
    uint32_t id;
    EnemyArchetypeId archetype;
    EnemySpawnType spawnType;

    sf::Vector2f position{};
    sf::Vector2f velocity;
    sf::Vector2f size;
    float pv = 2;
    bool active = true;
    float points = 10.f;
    float shootCooldown = 0.f;
    bool wantsToShoot = false;

    float waveAmplitude = 0.f;
    float waveFrequency = 0.f;
    float waveTime = 0.f;
    float baseY = 0.f;

    bool circleMove = false;
    sf::Vector2f circleCenter;
    float circleRadius = 0.f;
    float circleSpeed = 0.f;
    float circleAngle = 0.f;

    bool kamikaze = false;
    uint32_t targetPlayerId = 0;
    float kamikazeSpeed = 0.f;
    float kamikazeMaxSpeed = 400.f;
    float kamikazeTurnRate = 720.f;
    float kamikazeAcceleration = 200.f;
    float rotation = 0.f;

    Enemy(sf::Vector2f pos);
    void draw(Client &c, sf::RenderWindow &w);
    void update(float dt, Server &server);
    void updateKamikaze(Server &server, float dt);
    static TurretPlacement turretPlacementFromString(const std::string &s);
    static EnemyType enemyTypeFromString(const std::string &s);
    static EnemySpawnType spawnTypeFromString(const std::string &s);
};
