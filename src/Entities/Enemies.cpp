#include "Enemies.hpp"
#include "../Network/Client/Client.hpp"
#include "../Network/Server/Server.hpp"
#include <corecrt_math_defines.h>

Enemy::Enemy(sf::Vector2f pos)
    : position(pos) {}

void Enemy::draw(Client &c, sf::RenderWindow &w)
{
    auto it = c.visuals.find(archetype);
    if (it == c.visuals.end())
        return;

    if (it->second.texture.getSize().x == 0 || it->second.texture.getSize().y == 0)
    {
        std::cout << "Enemy texture empty for archetype " << (int)archetype << "\n";
        return;
    }
    sf::Sprite sprite(it->second.texture);
    float scaleX = size.x / it->second.texture.getSize().x;
    float scaleY = size.y / it->second.texture.getSize().y;
    float screenX = position.x - c.targetWorldX;
    float screenY = position.y;
    sprite.setPosition({screenX, screenY});
    sprite.setScale({scaleX, scaleY});
    w.draw(sprite);
}

void Enemy::update(float dt, Server &server)
{
    if (!active)
        return;

    if (spawnType != EnemySpawnType::Kamikaze)
        position += velocity * dt;

    switch (spawnType)
    {
    case EnemySpawnType::Simple:
    case EnemySpawnType::Group:
        break;

    case EnemySpawnType::Wave:
    {
        waveTime += dt;
        position.y = baseY +
                     std::sin(waveTime * waveFrequency) * waveAmplitude;
        break;
    }

    case EnemySpawnType::Circle:
    {
        if (circleMove)
        {
            circleAngle += circleSpeed * dt;

            position.x = circleCenter.x +
                         std::cos(circleAngle) * circleRadius;
            position.y = circleCenter.y +
                         std::sin(circleAngle) * circleRadius;
        }
        break;
    }

    case EnemySpawnType::Kamikaze:
    {
        updateKamikaze(server, dt);
        break;
    }

    default:
        break;
    }

    const auto &arche = EnemyArchetypeRegistry::get(archetype);

    shootCooldown -= dt;
    if (shootCooldown <= 0.f && arche.fireRate > 0.f)
    {
        shootCooldown += 1.f / arche.fireRate;
        if (position.x - server.worldX < Config::Get().windowSize.x)
            wantsToShoot = true;
    }

    if (position.x < server.worldX - 200.f)
        active = false;
}

void Enemy::updateKamikaze(Server &server, float dt)
{
    if (!active || targetPlayerId < 0)
        return;

    auto it = server.allPlayers.find(targetPlayerId);
    if (it == server.allPlayers.end() || !it->second.alive)
        return;

    sf::Vector2f targetPos = it->second.position;
    targetPos.x += server.worldX;

    sf::Vector2f toTarget = targetPos - position;
    float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
    if (dist < 0.01f)
        return;

    toTarget /= dist;

    float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    speed = std::min(speed + kamikazeAcceleration * dt, kamikazeMaxSpeed);

    sf::Vector2f dir = velocity;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.001f)
        dir = toTarget;
    else
        dir /= len;

    float dot = std::clamp(dir.x * toTarget.x + dir.y * toTarget.y, -1.f, 1.f);
    float angle = std::acos(dot);
    float maxTurn = kamikazeTurnRate * dt;
    float t = (angle > 0.f) ? std::min(1.f, maxTurn / angle) : 1.f;
    sf::Vector2f newDir = dir + (toTarget - dir) * t;
    float n = std::sqrt(newDir.x * newDir.x + newDir.y * newDir.y);
    if (n > 0.f)
        newDir /= n;

    velocity = newDir * speed;

    position += velocity * dt;

        rotation = std::atan2(velocity.y, velocity.x);
}

TurretPlacement Enemy::turretPlacementFromString(const std::string &s)
{
    if (s == "surface")
        return TurretPlacement::Surface;
    if (s == "bridge")
        return TurretPlacement::Bridge;
    if (s == "ceiling")
        return TurretPlacement::Ceiling;

    throw std::runtime_error("Unknown TurretPlacement: " + s);
}

EnemyType Enemy::enemyTypeFromString(const std::string &s)
{
    if (s == "drone")
        return EnemyType::DRONE;
    if (s == "turret")
        return EnemyType::TURRET;
    if (s == "kamikaze")
        return EnemyType::KAMIKAZE;
    if (s == "boss")
        return EnemyType::BOSS;

    throw std::runtime_error("Unknown EnemyType: " + s);
}

EnemySpawnType Enemy::spawnTypeFromString(const std::string &s)
{
    if (s == "simple")
        return EnemySpawnType::Simple;
    if (s == "circle")
        return EnemySpawnType::Circle;
    if (s == "kamikaze")
        return EnemySpawnType::Kamikaze;
    if (s == "group")
        return EnemySpawnType::Group;
    if (s == "boss")
        return EnemySpawnType::Boss;
    if (s == "wave")
        return EnemySpawnType::Wave;

    throw std::runtime_error("Unknown EnemySpawnType: " + s);
}