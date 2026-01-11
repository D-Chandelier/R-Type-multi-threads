#include "Server.hpp"

RemotePlayer *Server::findClosestAlivePlayer(const sf::Vector2f &pos)
{
    RemotePlayer *best = nullptr;
    float bestDistSq = (std::numeric_limits<float>::max)();

    for (auto &[id, p] : allPlayers)
    {
        if (!p.alive)
            continue;

        sf::Vector2f d = p.position - pos;
        float distSq = d.x * d.x + d.y * d.y;

        if (distSq < bestDistSq)
        {
            bestDistSq = distSq;
            best = &p;
        }
    }
    return best;
}

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

void Server::onEnemyDestroyed(Enemy &enemy, const sf::Vector2f &pos, RemotePlayer &killer)
{
    const auto &arche = EnemyArchetypeRegistry::get(enemy.archetype);

    if (arche.name == "turret")
    {
        bonusStats.turretsDestroyed++;

        if (bonusStats.turretsDestroyed % 5 == 0 && killer.score > 0)
        {
            Bonus::spawnBonus(BonusType::RocketX3, pos, *this);
        }
    }

    if (static_cast<int>(killer.score) % 200 == 0 && killer.score > 0)
    {
        Bonus::spawnBonus(BonusType::HealthX1, pos, *this);
    }

    if (static_cast<int>(killer.score) % 50 == 0 && killer.score > 0)
    {
        Bonus::spawnBonus(BonusType::FireRateUp, pos, *this);
    }
}
