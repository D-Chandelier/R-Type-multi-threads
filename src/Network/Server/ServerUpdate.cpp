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
        updateEnemies(SERVER_TICK);
        Bullet::updateBulletsServer(*this, SERVER_TICK);
        Bonus::updateBonusesServer(*this, SERVER_TICK);

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
        // if (allBonuses.size() > 0)
        // packetBroadcastBonuses();
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
                Enemy t(turretPos);
                t.type = EnemyType::TURRET;
                allEnemies.emplace(nextEnemyId++, t);
                packetBroadcastEnemies();
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

void Server::updateEnemies(float dt)
{
    for (auto it = allEnemies.begin(); it != allEnemies.end();)
    {
        Enemy &enemy = it->second;

        enemy.update(dt, worldX);

        if (!enemy.active)
        {
            it = allEnemies.erase(it);
            continue;
        }

        if (enemy.wantsToShoot)
        {
            // ToDo: spawnTurretBullet(enemy);
        }
        ++it;
    }
}

void Server::updateBonuses(float dt)
{
    bool anyBonusCollected = false;

    for (auto it = allBonuses.begin(); it != allBonuses.end();)
    {
        Bonus &bonus = it->second;

        // Hors écran (à gauche)
        if (bonus.position.x < worldX - 48.f)
        {
            bonus.active = false;
        }
        if (!bonus.active)
        {
            it = allBonuses.erase(it);
            continue;
        }

        bool collected = false;

        // Collision joueur ↔ bonus
        for (auto &[playerId, player] : allPlayers)
        {
            if (!player.alive)
                continue;

            sf::Vector2f d = player.position - bonus.position;
            constexpr float radius = 18.f; // rayon de ramassage

            if (d.x * d.x + d.y * d.y <= radius * radius)
            {
                applyBonus(player, bonus);
                collected = true;
                anyBonusCollected = true;
                break;
            }
        }

        if (collected)
        {
            // packetBroadcastBonusCollected(bonus.id);
            it = allBonuses.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (anyBonusCollected)
    {
        // packetBroadcastPlayers();
    }
}
