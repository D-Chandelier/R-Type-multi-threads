#include "Server.hpp"

void Server::killAndRespawn(RemotePlayer &p)
{
    p.pv--;
    p.alive = false;
    p.invulnerable = true;

    p.position =
        {
            Config::Get().playerArea.size.x * 0.5f,
            Config::Get().windowSize.y / 9.f * (Utils::getPlayerByPeer(p.peer, allPlayers)->id + 1.f)};

    p.velocity = {0.f, 0.f};

    const double now = Utils::currentGameTime(gameStartTime);
    p.respawnTime = now + 3.0; // timestamp absolu
}

void Server::spawnPlayerMissile(const sf::Vector2f &pos, uint32_t ownerId)
{
    Bullet m;
    m.id = nextBulletId++;
    m.position = pos;
    m.velocity = sf::Vector2f(MISSILE_START_SPEED, 0.f);
    m.damage = 4.f;
    m.ownerId = ownerId;
    m.type = BulletType::HOMING_MISSILE;
    m.lifetime = 0.f;
    m.targetId = 0;

    allBullets.emplace(m.id, m);
}

uint32_t Server::findClosestTarget(sf::Vector2f &from)
{
    uint32_t bestId = 0;
    float bestDistSq = MISSILE_RANGE * MISSILE_RANGE;

    sf::Vector2f newFrom = {from.x + worldX, from.y};

    for (auto &[id, enemy] : allEnemies) // ToDo: AllTurret vers AllEnemies
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
            spawnBonus(BonusType::RocketX3, pos);
        }
    }

    // ---- Score multiple de 100 ----
    if (static_cast<int>(killer.score) % 100 == 0)
    {
        spawnBonus(BonusType::HealthX1, pos);
    }

    // Extensions futures ici
}

void Server::spawnBonus(BonusType type, sf::Vector2f pos)
{
    Bonus b;
    b.type = type;
    b.position = pos;

    allBonuses.emplace(nextBonusId++, b);
    std::cout << "[BONUS] Spawn " << static_cast<int>(type) << " at {" << pos.x << ", " << pos.y << "}" << std::endl;
}

void Server::applyBonus(RemotePlayer &player, Bonus &bonus)
{
    switch (bonus.type)
    {
    case BonusType::RocketX3:
        player.nbRocket += 3;
        break;

    case BonusType::HealthX1:
        player.pv = min(player.pv + 1.f, player.maxPv);
        break;

    case BonusType::Shield:
        player.invulnerable = true;
        player.invulnTimer = 5.f;
        break;

    case BonusType::FireRateUp:
        player.fireRate *= 0.8f;
        break;

    case BonusType::ScoreBoost:
        player.score += 50;
        break;
    }

    bonus.active = false;
}
