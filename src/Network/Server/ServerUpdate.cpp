#include "Server.hpp"
#include "../../World/Entities.hpp"

void Server::update(float dt)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (!host)
        return;

    handleEnetService(dt);

    positionAccumulator += dt;

    while (positionAccumulator >= SERVER_TICK)
    {
        positionAccumulator -= SERVER_TICK;
        levelTick++;
        worldX += LevelRegistry::current()->scrollSpeed * SERVER_TICK;

        Segment::updateSegment(*this);
        updateEnemies(SERVER_TICK);
        Bullet::updateBulletsServer(*this, SERVER_TICK);
        Bonus::updateBonusesServer(*this, SERVER_TICK);

        const double now = Utils::currentGameTime(gameStartTime);

        for (auto &[id, p] : allPlayers)
        {
            if (p.invulnerable && now >= p.respawnTime)
            {
                p.invulnerable = false;
                p.alive = true;
            }
        }

        packetBroadcastWorldX();
        packetBroadcastPositions();
        packetBroadcastEnemies();
    }
}

void Server::updateEnemies(float dt)
{
    for (auto &e : runtimeEnemies)
    {
        if (e.spawned)
            continue;

        if (worldX + Config::Get().windowSize.x >= e.desc.atX - 64.f)
        {
            const auto &arche = EnemyArchetypeRegistry::get(e.desc.archetype);

            for (int i = 0; i < e.desc.count; i++)
            {
                Enemy enemy({e.desc.atX,
                             Config::Get().windowSize.y - e.desc.y * LevelRegistry::current()->tileSize.y});

                enemy.spawnType = e.desc.spawnType;

                if (e.desc.y == -1)
                {
                    std::uniform_int_distribution<int> dist(2, 9);
                    enemy.position.y = dist(terrain.rng) * LevelRegistry::current()->tileSize.y;
                }

                enemy.position.y -= arche.size.y;
                enemy.velocity = e.desc.velocity;

                switch (e.desc.spawnType)
                {
                case EnemySpawnType::Simple:
                    break;
                case EnemySpawnType::Wave:
                    enemy.waveAmplitude = 50.f;
                    enemy.waveFrequency = 2.f;
                    enemy.baseY = enemy.position.y;
                    break;
                case EnemySpawnType::Circle:
                    enemy.circleMove = true;
                    enemy.circleCenter = enemy.position;
                    enemy.circleRadius = 50.f;
                    enemy.circleSpeed = 3.f;
                    enemy.circleAngle = 0.f;
                    break;
                case EnemySpawnType::Group:
                    enemy.position.y += i * e.desc.spacing;
                    break;
                case EnemySpawnType::Kamikaze:
                {
                    enemy.spawnType = EnemySpawnType::Kamikaze;
                    enemy.kamikazeSpeed = std::sqrt(
                        e.desc.velocity.x * e.desc.velocity.x +
                        e.desc.velocity.y * e.desc.velocity.y);

                    RemotePlayer *target = findClosestAlivePlayer(enemy.position);
                    if (target)
                        enemy.targetPlayerId = target->id;

                                        break;
                }
                case EnemySpawnType::Boss:
                    enemy.velocity.x = 0.f;
                    break;
                }

                enemy.pv = arche.pv;
                enemy.points = arche.points;
                enemy.shootCooldown = (arche.fireRate > 0.f) ? 1.f / arche.fireRate : 0.f;
                enemy.archetype = e.desc.archetype;
                enemy.size = e.desc.size;

                allEnemies.emplace(nextEnemyId++, enemy);
            }
            e.spawned = true;
        }
    }

    for (auto it = allEnemies.begin(); it != allEnemies.end();)
    {
        Enemy &enemy = it->second;

        enemy.update(dt, *this);

        if (!enemy.active)
        {
            it = allEnemies.erase(it);
            continue;
        }

        if (enemy.wantsToShoot)
        {
            const auto &arche = EnemyArchetypeRegistry::get(enemy.archetype);
            if (arche.name == "turret")
                Bullet::spawnTurretBullet(enemy, *this);
            else
                Bullet::spawnBullet(enemy, *this);

            enemy.wantsToShoot = false;
        }
        ++it;
    }
}
