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
    m.velocity = sf::Vector2f(ROCKET_START_SPEED, 0.f);
    m.damage = 4.f;
    m.ownerId = ownerId;
    m.type = BulletType::HOMING_ROCKET;
    m.lifetime = 0.f;
    m.targetId = 0;

    allBullets.emplace(m.id, m);
}

uint32_t Server::findClosestTarget(sf::Vector2f &from)
{
    uint32_t bestId = 0;
    float bestDistSq = ROCKET_RANGE * ROCKET_RANGE;

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
    if (static_cast<int>(killer.score) % 200 == 0)
    {
        spawnBonus(BonusType::HealthX1, pos);
    }

    // ---- Score multiple de 100 ----
    if (static_cast<int>(killer.score) % 50 == 0)
    {
        spawnBonus(BonusType::FireRateUp, pos);
    }

    // Extensions futures ici
}

void Server::spawnBonus(BonusType type, sf::Vector2f pos)
{
    Bonus b;
    b.id = nextBonusId++;
    b.type = type;
    b.phase = randomFloat(0.f, 2.f * 3.14159265f);
    b.spawnPos = pos; // - sf::Vector2f{0.f, b.phase};
    b.position = pos;
    b.velocity = {Config::Get().speed * 0.75f, 0.f};
    b.time = 0.f;
    b.amplitude = Config::Get().windowSize.y * 0.35f;
    b.angularSpeed = 0.8f;
    allBonuses.emplace(b.id, b);

    // Envoi d’un packet spawn uniquement
    packetBroadcastBonusSpawn(b);
}

void Server::packetBroadcastRemoveBonus(uint32_t id)
{
    allBonuses.erase(id);
    packetBroadcastBonusDestroy(id);
}

float Server::randomFloat(float min, float max)
{
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

// void Server::spawnBonus(BonusType type, sf::Vector2f pos)
// {
//     Bonus b;
//     b.id = nextBonusId++;
//     b.type = type;
//     b.position = pos;
//     // Avance plus lentement qu’un bullet
//     b.velocity = {-40.f, 0.f};

//     allBonuses.emplace(b.id, b);

//     // std::cout << "spawnBonus: [" << b.id << "] {" << b.position.x << "," << b.position.y << "}\n";
//     // packetBroadcastBonuses();
// }

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
        player.fireRate *= 1.25f;
        break;

    case BonusType::ScoreBoost:
        player.score += 50;
        break;
    }

    bonus.active = false;
}
