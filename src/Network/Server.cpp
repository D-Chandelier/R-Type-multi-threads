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

    gameStartTime = getNowSeconds();
    std::cout << "[Server] started on port " << port << "at " << gameStartTime << "\n";
    serverReady = true;

    levelSeed = std::random_device{}();
    levelTick = 0;
    worldX = 0.f;

    terrain.init(levelSeed);
    // generateNextSegment();
    // generateNextSegment();
    while (terrain.nextSegmentX < worldX + lookahead)
        generateNextSegment();

    // Nettoyage optionnel
    while (!terrain.segments.empty() &&
           terrain.segments.front().startX + SEGMENT_WIDTH < worldX - cleanupMargin)
    {
        terrain.segments.pop_front();
    }
    // terrain.update(worldX);

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

double Server::getNowSeconds()
{
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

double Server::currentGameTime() const
{
    return getNowSeconds() - gameStartTime;
}

RemotePlayer *Server::getPlayerByPeer(ENetPeer *peer)
{
    for (auto &[id, player] : allPlayers)
    {
        if (player.peer == peer)
            return &player;
    }
    return nullptr;
}

inline bool aabbOverlap(const sf::FloatRect &a, const sf::FloatRect &b)
{
    return a.position.x < b.position.x + b.size.x &&
           a.position.x + a.size.x > b.position.x &&
           a.position.y < b.position.y + b.size.y &&
           a.position.y + a.size.y > b.position.y;
}

bool continuesGround(const TerrainSegment *prev)
{
    if (!prev)
        return false;
    if (prev->blocks.empty())
        return false;

    // Le dernier bloc du segment précédent touche le sol
    return prev->type == SegmentType::Flat ||
           prev->type == SegmentType::Corridor ||
           prev->type == SegmentType::TurretZone;
}

TerrainSegment Server::generateNextSegment()
{
    TerrainSegment seg;
    seg.startX = terrain.nextSegmentX;

    std::uniform_int_distribution<int> dist(0, 3);
    seg.type = static_cast<SegmentType>(dist(terrain.rng));

    const float groundY = Config::Get().windowSize.y - GROUND_HEIGHT;

    const int tilesX = static_cast<int>(SEGMENT_WIDTH / TILE);
    const int tilesY = static_cast<int>(GROUND_HEIGHT / TILE);

    auto groundVisual = [&](int x, int y)
    {
        if (y == 0)
        {
            if (x == 0)
                return BlockVisual::GroundTopLeft;
            if (x == tilesX - 1)
                return BlockVisual::GroundTopRight;
            return BlockVisual::GroundTopMid;
        }
        else
        {
            if (x == 0)
                return BlockVisual::GroundFillLeft;
            if (x == tilesX - 1)
                return BlockVisual::GroundFillRight;
            return BlockVisual::GroundFillMid;
        }
    };

    auto ceilingVisual = [&](int x)
    {
        if (x == 0)
            return BlockVisual::CeilingLeft;
        if (x == tilesX - 1)
            return BlockVisual::CeilingRight;
        return BlockVisual::CeilingMid;
    };

    switch (seg.type)
    {
    // =========================
    // FLAT
    // =========================
    case SegmentType::Flat:
    {
        //     constexpr float TILE = 64.f;
        //     const int tilesX = static_cast<int>(SEGMENT_WIDTH / TILE);
        //     const int tilesY = static_cast<int>(GROUND_HEIGHT / TILE);

        //     // Vérifier le segment précédent
        //     //const TerrainSegment *prev = terrain.segments.empty() ? nullptr : &terrain.segments.back();

        //     for (int y = 0; y < tilesY; ++y)
        //     {
        //         for (int x = 0; x < tilesX; ++x)
        //         {
        //             BlockVisual v;

        //             if (y == 0)
        //             {
        //                 // ligne du dessus
        //                 if (prev && prev->blocks.size() >= tilesY)
        //                 {
        //                     // // segment précédent présent : début = TopMid
        //                     // if (x == 0)
        //                     //     v = BlockVisual::GroundTopMid;
        //                     // else if (x == tilesX - 1)
        //                     //     v = BlockVisual::GroundTopRight;
        //                     // else
        //                     //     v = BlockVisual::GroundTopMid;
        //                 }
        //                 else
        //                 {
        //                     // pas de précédent
        //                     if (x == 0)
        //                         v = BlockVisual::GroundTopLeft;
        //                     else if (x == tilesX - 1)
        //                         v = BlockVisual::GroundTopRight;
        //                     else
        //                         v = BlockVisual::GroundTopMid;
        //                 }
        //             }
        //             else
        //             {
        //                 // lignes de remplissage
        //                 if (x == 0)
        //                     v = BlockVisual::GroundFillLeft;
        //                 else if (x == tilesX - 1)
        //                     v = BlockVisual::GroundFillRight;
        //                 else
        //                     v = BlockVisual::GroundFillMid;
        //             }

        //             seg.blocks.push_back({sf::FloatRect{{x * TILE, groundY + y * TILE}, {TILE, TILE}}, v});
        //         }
        //     }

        //     break;
        // }

        for (int y = 0; y < tilesY; ++y)
        {
            for (int x = 0; x < tilesX; ++x)
            {
                seg.blocks.push_back({sf::FloatRect{
                                          {x * TILE, groundY + y * TILE},
                                          {TILE, TILE}},
                                      groundVisual(x, y)});
            }
        }
        break;
    }

    // =========================
    // HOLE
    // =========================
    case SegmentType::Hole:
    {
        constexpr float TILE = 64.f;

        const float holeWidth = SEGMENT_WIDTH * 0.4f;
        const float sideWidth = (SEGMENT_WIDTH - holeWidth) * 0.5f;

        const int leftCount = static_cast<int>(sideWidth / TILE);
        const int rightCount = leftCount;

        // ----- Plateforme gauche -----
        for (int i = 0; i < leftCount; ++i)
        {
            BlockVisual v;
            if (i == 0)
                v = BlockVisual::GroundTopLeft;
            else if (i == leftCount - 1)
                v = BlockVisual::GroundTopRight; // bord du trou
            else
                v = BlockVisual::GroundTopMid;

            seg.blocks.push_back({sf::FloatRect{{i * TILE, groundY}, {TILE, TILE}},
                                  v});
        }

        // ----- Plateforme droite -----
        const float rightStartX = sideWidth + holeWidth;

        for (int i = 0; i < rightCount; ++i)
        {
            BlockVisual v;
            if (i == 0)
                v = BlockVisual::GroundTopLeft; // bord du trou
            else if (i == rightCount - 1)
                v = BlockVisual::GroundTopRight;
            else
                v = BlockVisual::GroundTopMid;

            seg.blocks.push_back({sf::FloatRect{{rightStartX + i * TILE, groundY}, {TILE, TILE}},
                                  v});
        }

        break;
    }

    // =========================
    // CORRIDOR
    // =========================
    case SegmentType::Corridor:
    {
        // sol
        for (int y = 0; y < tilesY; ++y)
        {
            for (int x = 0; x < tilesX; ++x)
            {
                seg.blocks.push_back({sf::FloatRect{
                                          {x * TILE, groundY + y * TILE},
                                          {TILE, TILE}},
                                      groundVisual(x, y)});
            }
        }

        // plafond
        for (int x = 0; x < tilesX; ++x)
        {
            seg.blocks.push_back({sf::FloatRect{
                                      {x * TILE, 0.f},
                                      {TILE, TILE}},
                                  ceilingVisual(x)});
        }
        break;
    }

    // =========================
    // TURRET ZONE (sol seul)
    // =========================
    case SegmentType::TurretZone:
    {
        for (int y = 0; y < tilesY; ++y)
        {
            for (int x = 0; x < tilesX; ++x)
            {
                seg.blocks.push_back({sf::FloatRect{
                                          {x * TILE, groundY + y * TILE},
                                          {TILE, TILE}},
                                      groundVisual(x, y)});
            }
        }
        break;
    }
    }

    terrain.segments.push_back(seg);
    terrain.nextSegmentX += SEGMENT_WIDTH - TILE;

    return seg;
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

// void Server::checkPlayerCollision(RemotePlayer &p)
// {
//     sf::FloatRect player = p.getBounds();
//     sf::Vector2f nextPos = p.position + p.velocity * SERVER_TICK;

//     // --- Correction X (gauche/droite)
//     for (const auto &seg : terrain.segments)
//     {
//         for (const auto &block : seg.blocks)
//         {
//             TerrainBlock b = block;
//             b.rect.position.x += seg.startX - worldX;

//             sf::FloatRect testRect = player;
//             testRect.position.x = nextPos.x;

//             if (testRect.findIntersection(b.rect))
//             {
//                 if (nextPos.x < b.rect.position.x)
//                     nextPos.x = b.rect.position.x - player.size.x; // bloque à gauche du bloc
//                 else
//                     nextPos.x = b.rect.position.x + b.rect.size.x; // bloque à droite du bloc

//                 p.velocity.x = 0.f;
//             }
//         }
//     }

//     // --- Correction Y (haut/bas)
//     for (const auto &seg : terrain.segments)
//     {
//         for (const auto &block : seg.blocks)
//         {
//             TerrainBlock b = block;
//             b.rect.position.x += seg.startX - worldX;

//             sf::FloatRect testRect = player;
//             testRect.position.y = nextPos.y;

//             if (testRect.findIntersection(b.rect))
//             {
//                 // collision par le bas
//                 if (p.velocity.y > 0 && player.position.y + player.size.y <= b.rect.position.y)
//                 {
//                     nextPos.y = b.rect.position.y - player.size.y;
//                     p.velocity.y = 0.f;
//                 }
//                 // collision par le haut
//                 else if (p.velocity.y < 0 && player.position.y >= b.rect.position.y + b.rect.size.y)
//                 {
//                     nextPos.y = b.rect.position.y + b.rect.size.y;
//                     p.velocity.y = 0.f;
//                 }
//             }
//         }
//     }

//     // --- Appliquer position finale
//     p.position = nextPos;
// }

void Server::killAndRespawn(RemotePlayer &p)
{
    p.alive = false;
    p.invulnerable = true;

    // Respawn au centre de l’écran
    p.position = {
        Config::Get().windowSize.x * 0.5f,
        Config::Get().windowSize.y * 0.5f};

    p.velocity = {0.f, 0.f};

    p.respawnTime = currentGameTime(); // temps serveur
}

void Server::sendLevel(ENetPeer *peer)
{
    InitLevelPacket pkt;
    pkt.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    pkt.header.code = static_cast<uint8_t>(ServerMsg::INIT_LEVEL);
    pkt.seed = levelSeed;
    pkt.worldX = worldX; // position monde actuelle
    pkt.scrollSpeed = LEVEL_SCROLL_SPEED;
    pkt.lookahead = lookahead;
    pkt.cleanupMargin = cleanupMargin;

    ENetPacket *packet = enet_packet_create(&pkt, sizeof(InitLevelPacket), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void Server::broadcastPositions()
{
    if (allPlayers.empty() || !host)
        return; // rien à broadcast

    ServerPositionPacket p;
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::PLAYER_POSITION);
    p.playerCount = static_cast<uint8_t>(allPlayers.size());

    p.serverGameTime = currentGameTime();
    p.scrollSpeed = BACKGROUND_SCROLL_SPEED;

    int i = 0;
    for (const auto &[id, player] : allPlayers)
    {
        p.players[i].id = player.id;
        p.players[i].x = player.position.x;
        p.players[i].y = player.position.y;
        p.players[i].alive = player.alive;
        p.players[i].invulnerable = player.invulnerable;
        p.players[i].respawnTime = player.respawnTime;

        i++;
    }

    for (const auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;
        ENetPacket *packet = enet_packet_create(&p, sizeof(ServerPositionPacket), 0);
        enet_peer_send(player.peer, 0, packet);
    }
}

void Server::broadcastBullets(const ServerBullet &b)
{
    ServerBulletPacket p;
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::BULLET_SHOOT);

    p.bulletId = b.id;
    p.x = b.position.x;
    p.y = b.position.y;
    p.velX = b.velocity.x;
    p.velY = b.velocity.y;
    p.ownerId = b.ownerId;

    for (auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;

        ENetPacket *pkt = enet_packet_create(
            &p,
            sizeof(p),
            ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(player.peer, 0, pkt);
    }
}

void Server::broadcastWorldX()
{
    if (allPlayers.empty() || !host)
        return; // rien à broadcast

    WorldStatePacket p;
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::WORLD_X_UPDATE); // Assurez-vous que ce code existe dans votre enum ServerMsg
    p.worldX = worldX;
    p.serverGameTime = currentGameTime();

    // Envoi du paquet à tous les joueurs connectés
    for (const auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;
        ENetPacket *packet = enet_packet_create(&p, sizeof(WorldStatePacket), 0);
        enet_peer_send(player.peer, 0, packet);
    }
}

void Server::sendAllSegments(ENetPeer *peer)
{
    const auto &segments = terrain.segments;

    size_t packetSize = sizeof(ServerAllSegmentsHeader);

    for (const auto &seg : segments)
        packetSize += sizeof(ServerSegmentPacket) +
                      seg.blocks.size() * sizeof(ServerAllSegmentsBlockPacket);

    ENetPacket *packet =
        enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_RELIABLE);

    uint8_t *ptr = packet->data;

    // Header
    ServerAllSegmentsHeader header;
    header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    header.code = static_cast<uint8_t>(ServerMsg::ALL_SEGMENTS);
    header.segmentCount = static_cast<uint16_t>(segments.size());

    memcpy(ptr, &header, sizeof(header));
    ptr += sizeof(header);

    // Segments
    for (const auto &seg : segments)
    {
        ServerSegmentPacket segPkt;
        segPkt.type = static_cast<uint8_t>(seg.type);
        segPkt.startX = seg.startX;
        segPkt.blockCount = static_cast<uint16_t>(seg.blocks.size());

        memcpy(ptr, &segPkt, sizeof(segPkt));
        ptr += sizeof(segPkt);

        for (const auto &b : seg.blocks)
        {
            ServerAllSegmentsBlockPacket blk;
            blk.x = b.rect.position.x;
            blk.y = b.rect.position.y;
            blk.w = b.rect.size.x;
            blk.h = b.rect.size.y;
            blk.visual = static_cast<uint8_t>(b.visual);
            memcpy(ptr, &blk, sizeof(blk));
            ptr += sizeof(blk);
        }
    }

    enet_peer_send(peer, 1, packet);
}

void Server::sendSegment(const TerrainSegment &seg, ENetPeer *peer)
{
    ServerSegmentPacket p;
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::NEW_SEGMENT); // Utilisez le code approprié pour l'envoi de segments
    p.type = static_cast<uint8_t>(seg.type);
    p.startX = seg.startX;
    p.blockCount = static_cast<uint8_t>(seg.blocks.size());
    for (size_t i = 0; i < seg.blocks.size(); ++i)
    {
        p.blocks[i].x = seg.blocks[i].rect.position.x;
        p.blocks[i].y = seg.blocks[i].rect.position.y;
        p.blocks[i].w = seg.blocks[i].rect.size.x;
        p.blocks[i].h = seg.blocks[i].rect.size.y;
        p.blocks[i].visual = static_cast<uint8_t>(seg.blocks[i].visual);
    }
    for (const auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;
        ENetPacket *packet = enet_packet_create(&p, sizeof(p), 0);
        enet_peer_send(peer, 1, packet); // channel 1 = terrain
    }
}

void Server::sendNewId(ENetEvent event)
{
    int newId = findFreePlayerId();
    ServerAssignIdPacket p;
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::ASSIGN_ID);
    p.serverStartTime = gameStartTime;

    if (newId != 100)
        allPlayers[newId] = RemotePlayer{.id = newId, .position = {Config::Get().playerArea.size.x * Config::Get().playerScale.x * 0.5f, Config::Get().windowSize.y / (Config::Get().maxPlayers + 1.f) * (newId + 1)}, .peer = event.peer};
    p.id = newId;
    event.peer->data = reinterpret_cast<void *>(static_cast<uintptr_t>(newId));

    ENetPacket *packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(event.peer, 0, packet);

    sendAllSegments(event.peer);
};

void Server::onReceivePlayerPosition(ENetEvent event, float dt)
{
    if (event.packet->dataLength != sizeof(ClientPositionPacket))
        return;
    auto *p = reinterpret_cast<ClientPositionPacket *>(event.packet->data);
    auto it = allPlayers.find(p->id);
    if (it == allPlayers.end())
        return;
    it->second.velocity = sf::Vector2f(p->velX, p->velY);
    checkPlayerCollision(it->second);
    it->second.position.x += p->velX * SERVER_TICK; // SERVER_TICK = dt côté serveur
    it->second.position.y += p->velY * SERVER_TICK;
}

void Server::onReceiveBulletShoot(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ClientBulletPacket))
        return;

    auto *p = reinterpret_cast<ClientBulletPacket *>(event.packet->data);

    RemotePlayer *shooter = getPlayerByPeer(event.peer);
    if (!shooter)
        return;

    sf::Vector2f dir{p->velX, p->velY};
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0.f)
        return;

    dir /= len;

    ServerBullet bullet;
    bullet.id = nextBulletId++;
    bullet.position = sf::Vector2f(shooter->getBounds().getCenter()); // sf::Vector2f{shooter->serverX, shooter->serverY}; // PAS p->x/p->y
    bullet.velocity = dir * shooter->bulletSpeed;
    bullet.damage = shooter->bulletDamage;
    bullet.ownerId = shooter->id;

    allBullets.emplace(bullet.id, bullet);
    broadcastBullets(bullet);
}

void Server::handleTypeConnect(ENetEvent event)
{
    sendNewId(event);
}

void Server::handleTypeReceive(ENetEvent event, float dt)
{
    if (!event.packet)
        return;

    PacketHeader *h = (PacketHeader *)event.packet->data;
    if (h->type != static_cast<uint8_t>(PacketType::CLIENT_MSG))
        return;
    switch (h->code)
    {
    case static_cast<uint8_t>(ClientMsg::PLAYER_POSITION):
        onReceivePlayerPosition(event, dt);
        break;
    case static_cast<uint8_t>(ClientMsg::BULLET_SHOOT):
        onReceiveBulletShoot(event);
        break;
    default:
        return;
    }
}

void Server::handleTypeDisconnect(ENetEvent event)
{
    std::cout << "Client déconnecté" << std::endl;
    for (auto it = allPlayers.begin(); it != allPlayers.end(); ++it)
    {
        if (it->second.peer == event.peer)
        {
            std::cout << "Suppression joueur id=" << it->first << std::endl;
            allPlayers.erase(it);
            break;
        }
    }
    event.peer->data = nullptr; // bonne pratique ENet
}

void Server::handleEnetService(float dt)
{
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            handleTypeConnect(event);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            handleTypeReceive(event, dt);
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            handleTypeDisconnect(event);
        default:
            break;
        }
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
            TerrainSegment seg = generateNextSegment();

            // Envoyer à tous les clients
            for (const auto &[id, player] : allPlayers)
            {
                if (!player.peer)
                    continue;
                sendSegment(seg, player.peer);
                // sendAllSegments(player.peer);
            }
        }

        for (auto &[id, p] : allPlayers)
        {
            if (p.invulnerable)
            {
                if (currentGameTime() - p.respawnTime >= 3.0)
                {
                    p.invulnerable = false;
                    p.alive = true;
                }
            }
        }

        broadcastWorldX();
        broadcastPositions();
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
