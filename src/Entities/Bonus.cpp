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

    // for (const auto &[id, b] : client.allBonuses)
    for (auto &b : activeBonuses)
    {
        // float angle = Bonus::bulletAngle(b.velocity) * 3.14159265f / 180.f;
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
    // Remplacer map client.allBonuses
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
    // p.x = b.position.x - client.targetWorldX;
    // p.y = b.position.y;

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

        // Trajectoire réelle
        b.position.x =
            b.spawnPos.x + b.velocity.x * b.time;

        b.position.y =
            b.spawnPos.y +
            std::sin(b.angularSpeed * b.time + b.phase) * b.amplitude;

        // Collision joueur ↔ bonus
        bool collected = false;
        for (auto &[pid, p] : server.allPlayers)
        {
            if (!p.alive)
                continue;

            if (checkCollision(b, p, server.worldX))
            {
                server.applyBonus(p, b);
                collected = true;
                break;
            }
        }

        // Hors écran
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
    // Hitbox joueur (monde)
    sf::FloatRect playerBox(
        {p.position.x + worldX, // + Config::Get().playerArea.size.x / 2,
         p.position.y},         //- Config::Get().playerArea.size.y / 2},
        {Config::Get().playerArea.size.x,
         Config::Get().playerArea.size.y});

    // Hitbox bonus (monde)
    sf::FloatRect bonusBox(
        {b.position.x,
         b.position.y},
        {BONUS_WIDTH,
         BONUS_HEIGHT});

    // Tolérance arcade
    constexpr float margin = 6.f;
    playerBox.position.x -= margin;
    playerBox.position.y -= margin;
    playerBox.size.x += margin * 2.f;
    playerBox.size.y += margin * 2.f;

    return playerBox.findIntersection(bonusBox).has_value();
}
