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

// void Server::spawnTurretBullet(const Enemy& turret)
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
    bool anyEnemiesDestroyed = false;

    for (auto it = allBullets.begin(); it != allBullets.end();)
    {
        bool destroyed = false;

        Bullet &b = it->second;

        if (b.type == BulletType::HOMING_MISSILE)
            updateMissile(b, dt);
        else
            b.position += b.velocity * dt;

        for (auto &[id, e] : allEnemies)
        {
            if (!e.active)
                continue;

            sf::Vector2f d = sf::Vector2f{b.position.x + worldX, b.position.y} - e.position;
            constexpr float radius = TURRET_HEIGHT / 2.f; // Ajuster selon le sprite
            if (d.x * d.x + d.y * d.y <= radius * radius)
            {
                b.active = false;
                destroyed = true;

                if (e.pv > 0)
                    e.pv -= b.damage;

                if (e.pv <= 0)
                {
                    allPlayers[b.ownerId].score += e.points;
                    e.active = false;
                    anyEnemiesDestroyed = true;
                    onEnemyDestroyed(EnemyType::TURRET, e.position, allPlayers[b.ownerId]);
                }
                break;
            }
        }

        if (destroyed || b.position.x > Config::Get().windowSize.x)
        {
            packetBroadcastBulletDestroyed(b.id);
            it = allBullets.erase(it);
        }

        else
            ++it;
    }

    if (anyEnemiesDestroyed)
    {

        packetBroadcastEnemies();
    }
}

void Server::updateMissile(Bullet &m, float dt)
{
    m.lifetime += dt;

    bool needBroadcast = true;

    // Phase 1 : lancement
    if (m.lifetime < MISSILE_LAUNCH_TIME)
    {
        packetBroadcastRocket(m);
        return;
    }

    // Acquisition cible (une seule fois)
    if (m.targetId == 0)
        m.targetId = findClosestTarget(m.position);

    float speed = std::sqrt(m.velocity.x * m.velocity.x + m.velocity.y * m.velocity.y);
    speed = min(speed + MISSILE_ACCELERATION * dt, MISSILE_MAX_SPEED);

    sf::Vector2f dir = m.velocity / speed;

    // Si pas de cible valide → ligne droite accélérée
    auto it = allEnemies.find(m.targetId);
    if (m.targetId == 0 || it == allEnemies.end() || !it->second.active)
    {
        m.velocity = dir * speed;
        m.position += m.velocity * dt;
        packetBroadcastRocket(m);
        return;
    }

    // Direction vers la cible
    // Position missile en monde
    sf::Vector2f missileWorldPos{
        m.position.x + worldX,
        m.position.y};

    // Direction vers la cible (monde → monde)
    sf::Vector2f toTarget = it->second.position - missileWorldPos;
    float len = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
    if (len > 0.f)
        toTarget /= len;

    // Rotation limitée
    float dot = dir.x * toTarget.x + dir.y * toTarget.y;
    dot = std::clamp(dot, -1.f, 1.f);

    float angle = std::acos(dot);
    float maxTurn = MISSILE_TURN_RATE * dt;

    float t = (angle > 0.f) ? min(1.f, maxTurn / angle) : 1.f;

    sf::Vector2f newDir = dir + (toTarget - dir) * t;
    float n = std::sqrt(newDir.x * newDir.x + newDir.y * newDir.y);
    if (n > 0.f)
        newDir /= n;

    m.velocity = newDir * speed;
    m.position += m.velocity * dt;

    packetBroadcastRocket(m);
}

void Server::updateBonuses(float dt)
{
    bool anyBonusCollected = false;

    for (auto it = allBonuses.begin(); it != allBonuses.end();)
    {
        Bonus &bonus = it->second;
        bool collected = false;

        if (!bonus.active)
        {
            it = allBonuses.erase(it);
            continue;
        }

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
