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

uint32_t Server::findClosestTarget(const sf::Vector2f &from)
{
    uint32_t bestId = 0;
    float bestDistSq = MISSILE_RANGE * MISSILE_RANGE;

    for (auto &[id, enemy] : allTurrets) // ToDo: AllTurret vers AllEnemies
    {
        if (!enemy.active)
            continue;

        sf::Vector2f d = enemy.position - from;
        float distSq = d.x * d.x + d.y * d.y;

        if (distSq < bestDistSq)
        {
            bestDistSq = distSq;
            bestId = id;
        }
    }
    return bestId;
}
