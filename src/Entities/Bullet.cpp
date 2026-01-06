#include "Bullet.hpp"
#include "../Network/Client/Client.hpp"
#include "../Network/Server/Server.hpp"

void Bullet::update(float dt)
{
    if (type == BulletType::HOMING_ROCKET)
        return;

    position += velocity * dt;

    // Exemple destruction hors écran
    if (position.x < -50 || position.x > Config::Get().windowSize.x + 50 ||
        position.y < -50 || position.y > Config::Get().windowSize.y + 50)
    {
        active = false;
    }
}

void Bullet::updateBulletsClient(Client &client, float dt)
{
    for (auto &[id, b] : client.allBullets)
        b.update(dt);

    std::erase_if(client.allBullets,
                  [](const auto &it)
                  { return !it.second.active; });

    sf::VertexArray &bVA = Get().bulletsVA;
    sf::VertexArray &rVA = Get().rocketsVA;
    bVA.clear();
    rVA.clear();

    bVA.setPrimitiveType(sf::PrimitiveType::Triangles);
    rVA.setPrimitiveType(sf::PrimitiveType::Triangles);

    for (const auto &[id, b] : client.allBullets)
    {
        float angle = bulletAngle(b.velocity) * 3.14159265f / 180.f;

        if (b.type == BulletType::HOMING_ROCKET)
        {
            Bullet::buildRocketQuad(b, angle, rVA);
        }
        else
        {
            Bullet::buildBulletQuad(b, angle, bVA);
        }
    }
}

void Bullet::buildBulletQuad(const Bullet &b, float angle, sf::VertexArray &bVA)
{
    float w = BULLET_WIDTH;
    float h = BULLET_HEIGHT;

    sf::Color color = BULLET_COLOR;

    sf::Vector2f local[4] =
        {
            {-w / 2.f, -h / 2.f},
            {w / 2.f, -h / 2.f},
            {w / 2.f, h / 2.f},
            {-w / 2.f, h / 2.f}};

    sf::Vector2f world[4];
    for (int i = 0; i < 4; ++i)
        world[i] = Bullet::rotatePoint(local[i], angle) + b.position;

    bVA.append({world[0], color});
    bVA.append({world[1], color});
    bVA.append({world[2], color});
    bVA.append({world[0], color});
    bVA.append({world[2], color});
    bVA.append({world[3], color});
}

void Bullet::buildRocketQuad(const Bullet &b, float angle, sf::VertexArray &rVA)
{
    float w = ROCKET_WIDTH;
    float h = ROCKET_HEIGHT;

    sf::Vector2f local[4] =
        {
            {-w / 2.f, -h / 2.f},
            {w / 2.f, -h / 2.f},
            {w / 2.f, h / 2.f},
            {-w / 2.f, h / 2.f}};

    sf::Vector2f world[4];
    for (int i = 0; i < 4; ++i)
        world[i] = Bullet::rotatePoint(local[i], angle) + b.position;

    sf::Vector2f uv[4] =
        {
            {0.f, 0.f},
            {w, 0.f},
            {w, h},
            {0.f, h}};

    rVA.append({world[0], sf::Color::White, uv[0]});
    rVA.append({world[1], sf::Color::White, uv[1]});
    rVA.append({world[2], sf::Color::White, uv[2]});
    rVA.append({world[0], sf::Color::White, uv[0]});
    rVA.append({world[2], sf::Color::White, uv[2]});
    rVA.append({world[3], sf::Color::White, uv[3]});
}

void Bullet::drawBullets(sf::RenderWindow &w)
{
    sf::VertexArray &bVA = Get().bulletsVA;
    sf::VertexArray &rVA = Get().rocketsVA;

    if (bVA.getVertexCount() > 0)
    {
        sf::RenderStates s;
        s.texture = nullptr;
        w.draw(bVA, s);
    }

    if (rVA.getVertexCount() > 0)
    {
        sf::RenderStates s;
        s.texture = &Config::Get().rocketTexture;
        w.draw(rVA, s);
    }
}

void Bullet::updateBulletsServer(Server &server, float dt)
{
    bool anyEnemiesDestroyed = false;

    for (auto it = server.allBullets.begin(); it != server.allBullets.end();)
    {
        bool destroyed = false;

        Bullet &b = it->second;

        if (b.type == BulletType::HOMING_ROCKET)
            updateRocketServer(server, b, dt);
        else
            b.position += b.velocity * dt;

        for (auto &[id, e] : server.allEnemies)
        {
            if (!e.active)
                continue;

            sf::Vector2f d = sf::Vector2f{b.position.x + server.worldX, b.position.y} - e.position;
            constexpr float radius = TURRET_HEIGHT / 2.f; // Ajuster selon le sprite
            if (d.x * d.x + d.y * d.y <= radius * radius)
            {
                b.active = false;
                destroyed = true;

                if (e.pv > 0)
                    e.pv -= b.damage;

                if (e.pv <= 0)
                {
                    server.allPlayers[b.ownerId].score += e.points;
                    e.active = false;
                    anyEnemiesDestroyed = true;
                    server.onEnemyDestroyed(EnemyType::TURRET, e.position, server.allPlayers[b.ownerId]);
                }
                break;
            }
        }

        if (destroyed || b.position.x > Config::Get().windowSize.x)
        {
            server.packetBroadcastBulletDestroyed(b.id);
            it = server.allBullets.erase(it);
        }

        else
            ++it;
    }

    if (anyEnemiesDestroyed)
    {

        server.packetBroadcastEnemies();
    }
}

void Bullet::updateRocketServer(Server &server, Bullet &m, float dt)
{
    m.lifetime += dt;

    bool needBroadcast = true;

    // Phase 1 : lancement
    if (m.lifetime < ROCKET_LAUNCH_TIME)
    {
        server.packetBroadcastRocket(m);
        return;
    }

    // Acquisition cible (une seule fois)
    if (m.targetId == 0)
        m.targetId = server.findClosestTarget(m.position);

    float speed = std::sqrt(m.velocity.x * m.velocity.x + m.velocity.y * m.velocity.y);
    speed = std::min(speed + ROCKET_ACCELERATION * dt, ROCKET_MAX_SPEED);

    sf::Vector2f dir = m.velocity / speed;

    // Si pas de cible valide → ligne droite accélérée
    auto it = server.allEnemies.find(m.targetId);
    if (m.targetId == 0 || it == server.allEnemies.end() || !it->second.active)
    {
        m.velocity = dir * speed;
        m.position += m.velocity * dt;
        server.packetBroadcastRocket(m);
        return;
    }

    // Direction vers la cible
    // Position missile en monde
    sf::Vector2f missileWorldPos{
        m.position.x + server.worldX,
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
    float maxTurn = ROCKET_TURN_RATE * dt;

    float t = (angle > 0.f) ? std::min(1.f, maxTurn / angle) : 1.f;

    sf::Vector2f newDir = dir + (toTarget - dir) * t;
    float n = std::sqrt(newDir.x * newDir.x + newDir.y * newDir.y);
    if (n > 0.f)
        newDir /= n;

    m.velocity = newDir * speed;
    m.position += m.velocity * dt;

    server.packetBroadcastRocket(m);
}