#include "Bonus.hpp"
#include "../Network/Client/Client.hpp"
#include "../Network/Server/Server.hpp"

void Bonus::updateBonusesClient(Client &client, float dt)
{
    std::lock_guard<std::mutex> lock(client.bonusesMutex);

    std::vector<Bonus> activeBonuses;
    activeBonuses.reserve(client.allBonuses.size());

    for (auto &[id, b] : client.allBonuses)
    {
        b.time += dt;

        b.position.x =
            b.spawnPos.x - client.targetWorldX + b.velocity.x * b.time;

        b.position.y =
            b.spawnPos.y +
            std::sin(b.angularSpeed * b.time + b.phase) * b.amplitude;

        if (b.position.x >= -50)
            activeBonuses.push_back(b);
    }

    sf::VertexArray &rVA = Get().RocketX3VA;
    sf::VertexArray &hVA = Get().HealthX1VA;
    sf::VertexArray &sVA = Get().ShieldVA;
    sf::VertexArray &fVA = Get().FireRateUpVA;
    sf::VertexArray &scVA = Get().ScoreBoostVA;
    rVA.clear();
    hVA.clear();
    sVA.clear();
    fVA.clear();
    scVA.clear();
    rVA.setPrimitiveType(sf::PrimitiveType::Triangles);
    hVA.setPrimitiveType(sf::PrimitiveType::Triangles);
    sVA.setPrimitiveType(sf::PrimitiveType::Triangles);
    fVA.setPrimitiveType(sf::PrimitiveType::Triangles);
    scVA.setPrimitiveType(sf::PrimitiveType::Triangles);

    for (auto &b : activeBonuses)
    {
        switch (b.type)
        {
        case BonusType::RocketX3:
            buildBonusQuad(b, rVA);
            break;
        case BonusType::HealthX1:
            buildBonusQuad(b, hVA);
            break;
        case BonusType::Shield:
            buildBonusQuad(b, sVA);
            break;
        case BonusType::FireRateUp:
            buildBonusQuad(b, fVA);
            break;
        case BonusType::ScoreBoost:
            buildBonusQuad(b, scVA);
            break;
        default:
            break;
        }
    }
    std::unordered_map<uint32_t, Bonus> newMap;
    for (auto &b : activeBonuses)
        newMap.emplace(b.id, b);

    client.allBonuses.swap(newMap);
}

void Bonus::buildBonusQuad(const Bonus &b, sf::VertexArray &va)
{
    constexpr float w = BONUS_WIDTH;
    constexpr float h = BONUS_WIDTH;
    sf::Color color = BONUS_COLOR;

    switch (b.type)
    {
    case BonusType::RocketX3:
    case BonusType::HealthX1:
    case BonusType::FireRateUp:
        color = sf::Color::White;
        break;
    default:
        break;
    }

    sf::Vector2f p = b.position;

    sf::Vector2f v0 = {p.x - w * 0.5f, p.y - h * 0.5f};
    sf::Vector2f v1 = {p.x + w * 0.5f, p.y - h * 0.5f};
    sf::Vector2f v2 = {p.x + w * 0.5f, p.y + h * 0.5f};
    sf::Vector2f v3 = {p.x - w * 0.5f, p.y + h * 0.5f};

    sf::Vector2f uv0 = {0.f, 0.f};
    sf::Vector2f uv1 = {w, 0.f};
    sf::Vector2f uv2 = {w, h};
    sf::Vector2f uv3 = {0.f, h};

    va.append({v0, color, uv0});
    va.append({v1, color, uv1});
    va.append({v2, color, uv2});

    va.append({v0, color, uv0});
    va.append({v2, color, uv2});
    va.append({v3, color, uv3});
}

void Bonus::drawBonuses(sf::RenderWindow &w)
{
    sf::VertexArray &rVA = Get().RocketX3VA;
    sf::VertexArray &hVA = Get().HealthX1VA;
    sf::VertexArray &sVA = Get().ShieldVA;
    sf::VertexArray &fVA = Get().FireRateUpVA;
    sf::VertexArray &scVA = Get().ScoreBoostVA;

    if (rVA.getVertexCount() > 0)
    {
        sf::RenderStates s;
        s.texture = &Config::Get().RocketX3Tex;
        w.draw(rVA, s);
    }
    if (hVA.getVertexCount() > 0)
    {
        sf::RenderStates s;
        s.texture = &Config::Get().HealthX1Tex;
        w.draw(hVA, s);
    }
    if (sVA.getVertexCount() > 0)
    {
        sf::RenderStates s;
        s.texture = nullptr;
        w.draw(sVA, s);
    }
    if (fVA.getVertexCount() > 0)
    {
        sf::RenderStates s;
        s.texture = &Config::Get().FireRateUpTex;
        w.draw(fVA, s);
    }
    if (scVA.getVertexCount() > 0)
    {
        sf::RenderStates s;
        s.texture = nullptr;
        w.draw(scVA, s);
    }
}

void Bonus::updateBonusesServer(Server &server, float dt)
{
    for (auto it = server.allBonuses.begin(); it != server.allBonuses.end();)
    {
        Bonus &b = it->second;

        b.time += dt;

        b.position.x =
            b.spawnPos.x + b.velocity.x * b.time;

        b.position.y =
            b.spawnPos.y +
            std::sin(b.angularSpeed * b.time + b.phase) * b.amplitude;

        bool collected = false;
        for (auto &[pid, p] : server.allPlayers)
        {
            if (!p.alive)
                continue;

            if (checkCollision(b, p, server.worldX))
            {
                applyBonus(p, b);
                collected = true;
                break;
            }
        }

        if (collected || b.position.x - server.worldX <= -BONUS_WIDTH)
        {
            uint32_t destroyedId = b.id;
            it = server.allBonuses.erase(it);
            server.packetBroadcastBonusDestroy(destroyedId);
        }
        else
            ++it;
    }
}

bool Bonus::checkCollision(const Bonus &b, const RemotePlayer &p, float worldX)
{
    sf::FloatRect playerBox(p.getBounds());
    playerBox.position.x += worldX;

    sf::FloatRect bonusBox(
        {b.position.x,
         b.position.y},
        {BONUS_WIDTH,
         BONUS_HEIGHT});

    constexpr float margin = 36.f;
    playerBox.position.y += margin;
    playerBox.size.y -= margin;

    return playerBox.findIntersection(bonusBox).has_value();
}

void Bonus::spawnBonus(BonusType type, sf::Vector2f pos, Server &server)
{
    Bonus b;
    b.id = server.nextBonusId++;
    b.type = type;
    b.phase = randomFloat(0.f, 2.f * 3.14159265f);
    b.spawnPos = pos;
    b.position = pos;
    b.velocity = {Config::Get().speed * 1.75f, 0.f};
    b.time = 0.f;
    b.amplitude = (Config::Get().windowSize.y - pos.y);
    b.angularSpeed = 0.8f;
    server.allBonuses.emplace(b.id, b);

    server.packetBroadcastBonusSpawn(b);
}

void Bonus::applyBonus(RemotePlayer &player, Bonus &bonus)
{
    switch (bonus.type)
    {
    case BonusType::RocketX3:
        player.nbRocket += 3;
        break;

    case BonusType::HealthX1:
        player.pv = std::min(player.pv + 1.f, player.maxPv);
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

float Bonus::randomFloat(float min, float max)
{
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}