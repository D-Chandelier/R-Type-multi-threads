#include "Server.hpp"

uint32_t Server::findClosestTarget(sf::Vector2f &from)
{
    uint32_t bestId = 0;
    float bestDistSq = ROCKET_RANGE * ROCKET_RANGE;

    sf::Vector2f newFrom = {from.x + worldX, from.y};

    for (auto &[id, enemy] : allEnemies)
    {
        if (!enemy.active)
            continue;

        sf::Vector2f d = enemy.position - newFrom;
        float distSq = d.x * d.x + d.y * d.y;

        if (distSq < bestDistSq)
        {
            bestDistSq = distSq;
            bestId = id;
        }
    }
    return bestId;
}

void Server::onEnemyDestroyed(EnemyType enemyType, const sf::Vector2f &pos, RemotePlayer &killer)
{
    // ---- Enemy détruite ----
    if (enemyType == EnemyType::TURRET)
    {
        bonusStats.turretsDestroyed++;

        if (bonusStats.turretsDestroyed % 5 == 0)
        {
            Bonus::spawnBonus(BonusType::RocketX3, pos, *this);
        }
    }

    // ---- Score multiple de 200 ----
    if (static_cast<int>(killer.score) % 200 == 0)
    {
        Bonus::spawnBonus(BonusType::HealthX1, pos, *this);
    }

    // ---- Score multiple de 50 ----
    if (static_cast<int>(killer.score) % 50 == 0)
    {
        Bonus::spawnBonus(BonusType::FireRateUp, pos, *this);
    }

    // Extensions futures ici
}
