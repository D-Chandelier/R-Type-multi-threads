#include "Server.hpp"

Server::Server() : host(nullptr), positionAccumulator(0.0), lookahead(), cleanupMargin()
{
    lookahead = 3.f * Config::Get().windowSize.x;
    cleanupMargin = 2.f * Config::Get().windowSize.x;
}

Server::~Server() { stop(); }

bool Server::start(uint16_t port)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (host)
        return true;

    allPlayers.clear();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    host = enet_host_create(&address, 32, 2, 0, 0);
    if (!host)
        return false;

    gameStartTime = Utils::localTimeNow();
    std::cout << "[Server] started on port " << port << "at " << gameStartTime << "\n";
    serverReady = true;

    levelSeed = std::random_device{}();
    levelTick = 0;
    worldX = 0.f;

    terrain.init(levelSeed);
    while (terrain.nextSegmentX < worldX + lookahead)
        Segments::generateNextSegment(terrain);

    // Nettoyage optionnel
    while (!terrain.segments.empty() &&
           terrain.segments.front().startX + SEGMENT_WIDTH < worldX - cleanupMargin)
    {
        terrain.segments.pop_front();
    }

    return true;
}

void Server::stop()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (host)
    {
        enet_host_flush(host);
        enet_host_destroy(host);
        host = nullptr;
    }
}

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

        // Génération segments si nécessaire
        while (terrain.nextSegmentX < worldX + lookahead)
        {
            // TerrainSegment seg = Terrain::generateNextSegment();
            TerrainSegment seg = Segments::generateNextSegment(terrain);

            // Envoyer à tous les clients
            for (const auto &[id, player] : allPlayers)
            {
                if (!player.peer)
                    continue;
                packetSendSegment(seg, player.peer);
                // sendAllSegments(player.peer);
            }
        }

        for (auto &[id, p] : allPlayers)
        {
            if (p.invulnerable)
            {
                if (Utils::currentGameTime(gameStartTime) - p.respawnTime >= 3.0)
                {
                    p.invulnerable = false;
                    p.alive = true;
                }
            }
        }

        updateTurrets(SERVER_TICK);

        packetBroadcastWorldX();
        packetBroadcastPositions();
    }
}

void Server::updateTurrets(float dt)
{
    for (auto &seg : terrain.segments)
    {
        for (auto &turret : seg.turrets)
        {
            if (!turret.active)
                continue;

            turret.shootCooldown -= dt;
            if (turret.shootCooldown <= 0.f)
            {
                // shootFromTurret(turret);
                turret.shootCooldown = 1.f; // 1 seconde entre tirs par exemple
            }
        }
    }
}

int Server::findFreePlayerId()
{
    for (uint32_t id = 0; id < Config::Get().maxPlayers; ++id)
    {
        if (allPlayers.find(id) == allPlayers.end())
            return id;
    }
    return 100; // aucun slot libre
}

void Server::checkPlayerCollision(RemotePlayer &p)
{
    bool blockedX = false;
    bool blockedY = false;

    // 1. Appliquer le mouvement
    p.position += p.velocity * SERVER_TICK;

    sf::FloatRect player = p.getBounds();

    for (const auto &seg : terrain.segments)
    {
        for (const auto &block : seg.blocks)
        {
            TerrainBlock b = block;
            b.rect.position.x += seg.startX - worldX; // ⚠️ ABSOLU, PAS worldX

            if (!b.rect.findIntersection(player))
                continue;

            // Overlaps (MTV)
            float dx1 = b.rect.position.x + b.rect.size.x - player.position.x;
            float dx2 = player.position.x + player.size.x - b.rect.position.x;
            float dy1 = b.rect.position.y + b.rect.size.y - player.position.y;
            float dy2 = player.position.y + player.size.y - b.rect.position.y;

            float overlapX = min(dx1, dx2);
            float overlapY = min(dy1, dy2);

            // Résolution sur l’axe le plus faible
            if (overlapX < overlapY)
            {
                if (player.position.x < b.rect.position.x)
                    p.position.x -= overlapX;
                else
                    p.position.x += overlapX;

                p.velocity.x = 0.f;

                blockedX = true;
            }
            else
            {
                if (player.position.y < b.rect.position.y)
                    p.position.y -= overlapY; // petit fudge pour éviter de rester collé
                else
                    p.position.y += overlapY;

                p.velocity.y = 0.f;
                // blockedY = true;
            }
            // Clamp final écran (optionnel)
            p.position.x = std::clamp(
                p.position.x,
                0.f,
                static_cast<float>(Config::Get().windowSize.x) - player.size.x);

            p.position.y = std::clamp(
                p.position.y,
                0.f,
                static_cast<float>(Config::Get().windowSize.y) - player.size.y);

            // Mettre à jour le rect après correction
            player = p.getBounds();
            if (player.findIntersection(b.rect))

                if (blockedX && p.position.x <= 0.f && !p.invulnerable)
                {
                    killAndRespawn(p);
                    return;
                }
        }
    }
    p.position.x = std::clamp(
        p.position.x,
        0.f,
        static_cast<float>(Config::Get().windowSize.x) - player.size.x);

    p.position.y = std::clamp(
        p.position.y,
        0.f,
        static_cast<float>(Config::Get().windowSize.y) - player.size.y);
}

void Server::killAndRespawn(RemotePlayer &p)
{
    p.alive = false;
    p.invulnerable = true;

    // Respawn au centre de l’écran
    p.position = {
        Config::Get().playerArea.size.x * 0.5f,
        Config::Get().windowSize.y / 9.f * (Utils::getPlayerByPeer(p.peer, allPlayers)->id + 1.f)};

    p.velocity = {0.f, 0.f};

    p.respawnTime = Utils::currentGameTime(gameStartTime); // temps serveur
}