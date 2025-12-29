#include "Server.hpp"

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
        worldX += LEVEL_SCROLL_SPEED * SERVER_TICK;

        // updateSegment(dt);
        updateSegment();
        updateTurrets(SERVER_TICK);
        updateBullets(SERVER_TICK);

        // Respawn / invulnérabilité
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
        // packetBroadcastTurrets();
    }
}
// Génération de segments et spawn des turrets
void Server::updateSegment()
{
    while (terrain.nextSegmentX < worldX + lookahead)
    {
        TerrainSegment seg = Segments::generateNextSegment(terrain);

        // Créer les tourelles pour les blocs autorisés
        for (auto &blk : seg.blocks)
        {
            if (blk.hasTurret)
            {
                sf::Vector2f turretPos{
                    seg.startX + blk.rect.position.x + TILE / 2.f,
                    blk.rect.position.y - TURRET_HEIGHT / 2.f}; // - TILE / 2.f };
                allTurrets.emplace(nextTurretId++, Turret(turretPos));
                packetBroadcastTurrets();
            }
        }

        // Envoyer le segment aux clients
        for (const auto &[id, player] : allPlayers)
        {
            if (!player.peer)
                continue;
            packetSendSegment(seg, player.peer);
        }
    }
}

// Mise à jour des turrets : scroll, cooldown, tir
void Server::updateTurrets(float dt)
{
    for (auto it = allTurrets.begin(); it != allTurrets.end();)
    {
        Turret &turret = it->second;

        if (!turret.active)
        {
            it = allTurrets.erase(it);
            continue;
        }

        // 2) Suppression hors écran (côté gauche)
        if (turret.position.x < worldX - TURRET_WIDTH)
        {
            it = allTurrets.erase(it);
            continue;
        }

        // 3) Cooldown de tir
        turret.shootCooldown -= dt;
        if (turret.shootCooldown <= 0.f)
        {
            turret.shootCooldown += TURRET_FIRE_INTERVAL;

            // ToDo:
            // spawnTurretBullet(turret);
        }

        ++it;
    }
}
// void Server::spawnTurretBullet(const Turret& turret)
// {
//     ServerBullet b;
//     b.position = turret.position + sf::Vector2f{-20.f, 0.f};
//     b.velocity = sf::Vector2f{-BULLET_SPEED, 0.f};
//     b.owner = BulletOwner::TURRET;

//     allBullets.emplace(b.id, b);
// }

// Mise à jour des bullets et collisions avec turrets
void Server::updateBullets(float dt)
{
    bool anyTurretDestroyed = false;

    for (auto it = allBullets.begin(); it != allBullets.end();)
    {
        bool destroyed = false;

        ServerBullet &b = it->second;

        b.position += b.velocity * dt;

        for (auto &[turretId, turret] : allTurrets)
        {
            if (!turret.active)
                continue;

            sf::Vector2f d = sf::Vector2f{b.position.x + worldX, b.position.y} - turret.position;
            constexpr float radius = TURRET_HEIGHT / 2.f; // Ajuster selon le sprite
            if (d.x * d.x + d.y * d.y <= radius * radius)
            {
                b.active = false;
                destroyed = true;

                if (turret.pv > 0)
                    turret.pv -= allPlayers[b.ownerId].bulletDamage;

                if (turret.pv <= 0)
                {
                    allPlayers[b.ownerId].score += turret.points;
                    turret.active = false;
                    anyTurretDestroyed = true;
                }
                break;
            }
        }
        // for (auto seg = terrain.segments.begin(); seg != terrain.segments.end();)
        // {
        //     for (auto &block : seg->blocks)
        //     {
        //         sf::Vector2f d = sf::Vector2f{b.position.x + worldX, b.position.y};
        //         if (block.rect.contains(b.position))
        //         {
        //             destroyed = true;
        //             break;
        //         }
        //     }
        // }

        if (destroyed || b.position.x > Config::Get().windowSize.x)
        {
            packetBroadcastBulletDestroyed(b.id);
            it = allBullets.erase(it);
        }
        else
            ++it;
    }

    if (anyTurretDestroyed)
    {
        packetBroadcastTurrets();
    }
}
