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

        Bullet &b = it->second;

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
        if (b.type == BulletType::HOMING_MISSILE)
            updateMissile(b, dt);
        else
            b.position += b.velocity * dt;

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

void Server::updateMissile(Bullet &m, float dt)
{
    m.lifetime += dt;

    // Phase 1 : lancement
    if (m.lifetime < MISSILE_LAUNCH_TIME)
    {
        m.position += m.velocity * dt;
        return;
    }

    // Phase 2 : acquisition cible
    if (m.targetId == 0)
    {
        m.targetId = findClosestTarget(m.position);
    }

    // Accélération
    float speed = std::sqrt(m.velocity.x * m.velocity.x + m.velocity.y * m.velocity.y);
    speed = min(speed + MISSILE_ACCELERATION * dt, MISSILE_MAX_SPEED);

    // Si pas de cible → ligne droite accélérée
    if (m.targetId == 0 || !allTurrets[m.targetId].active) // ToDo: allTurret vers allEnemies
    {
        sf::Vector2f dir = m.velocity / speed;
        m.velocity = dir * speed;
        m.position += m.velocity * dt;
        return;
    }

    // Direction actuelle
    sf::Vector2f dir = m.velocity / speed;

    // Direction vers la cible
    sf::Vector2f toTarget = allTurrets[m.targetId].position - m.position; // ToDo: allTurret vers allEnemies
    float len = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
    toTarget /= len;

    // Interpolation angulaire (effet missile)
    float dot = dir.x * toTarget.x + dir.y * toTarget.y;
    dot = std::clamp(dot, -1.f, 1.f);
    float angle = std::acos(dot);

    float maxTurn = MISSILE_TURN_RATE * dt;
    float t = min(1.f, maxTurn / angle);

    sf::Vector2f newDir = dir + (toTarget - dir) * t;
    float n = std::sqrt(newDir.x * newDir.x + newDir.y * newDir.y);
    newDir /= n;

    m.velocity = newDir * speed;
    m.position += m.velocity * dt;
}
