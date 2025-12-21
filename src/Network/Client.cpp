#include "Client.hpp"

Client::Client()
    : clientHost(nullptr),
      peer(nullptr),
      ConnexionState(ClientState::DISCONNECTED),
      terrain() {}

Client::~Client() { stop(); }

bool Client::init()
{
    if (clientHost)
        return true; // déjà créé
    clientHost = enet_host_create(nullptr, 1, 2, 0, 0);
    return clientHost != nullptr;
}

double Client::localTimeNow() const
{
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

bool Client::connectTo(const char *host, uint16_t port)
{
    init();
    if (!clientHost)
        return false;

    ENetAddress address;
    enet_address_set_host(&address, host);
    address.port = port;

    peer = enet_host_connect(clientHost, &address, 2, 0);
    if (!peer)
        return false;

    ConnexionState = ClientState::CONNECTING;
    std::cout << "[Client] connecting to " << host << ":" << port << "\n";
    return true;
}

void Client::stop()
{
    if (peer)
    {
        enet_host_flush(clientHost);
        enet_peer_disconnect(peer, 0);
        peer = nullptr;
    }

    if (clientHost)
    {
        enet_host_flush(clientHost);
        enet_host_destroy(clientHost);
        clientHost = nullptr;
    }
}

void Client::onReceiveSegment(ENetEvent &event)
{
    if (event.packet->dataLength != sizeof(ServerSegmentPacket))
        return;

    auto *p = reinterpret_cast<ServerSegmentPacket *>(event.packet->data);

    TerrainSegment seg;
    seg.type = static_cast<SegmentType>(p->type);
    seg.startX = p->startX;

    for (int i = 0; i < p->blockCount; ++i)
    {
        const auto &b = p->blocks[i];
        seg.blocks.emplace_back(
            sf::FloatRect{{b.x, b.y}, {b.w, b.h}},
            static_cast<BlockVisual>(b.visual));
    }

    terrain.segments.push_back(seg);
}

void Client::eventReceivePlayersPositions(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ServerPositionPacket))
        return;

    auto *p = reinterpret_cast<ServerPositionPacket *>(event.packet->data);

    std::unordered_set<int> seenIds;

    for (int i = 0; i < p->playerCount; i++)
    {
        int pid = p->players[i].id;

        seenIds.insert(pid);

        RemotePlayer &rp = allPlayers[pid];
        rp.id = pid;
        rp.lastUpdateTime = p->serverGameTime;
        rp.serverPosition.x = p->players[i].x;
        rp.serverPosition.y = p->players[i].y;
        rp.alive = p->alive;
        rp.invulnerable = p->invulnerable;
        rp.respawnTime = p->respawnTime;
    }

    // Supprimer uniquement les joueurs distants qui ne sont plus dans le snapshot
    for (auto it = allPlayers.begin(); it != allPlayers.end();)
    {
        // if (it->first != localPlayer.id && seenIds.find(it->first) == seenIds.end())
        if (it->first != Config::Get().playerId && seenIds.find(it->first) == seenIds.end())
            it = allPlayers.erase(it);
        else
            ++it;
    }

    serverTimeOffset = p->serverGameTime - localTimeNow();
    backgroundScrollSpeed = p->scrollSpeed;
}

void Client::eventReceiveBullets(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ServerBulletPacket))
        return;

    auto *p = reinterpret_cast<ServerBulletPacket *>(event.packet->data);

    // Anti-duplication (important)
    if (allBullets.contains(p->bulletId))
        return;

    Bullet b;
    b.id = p->bulletId;
    b.position = {p->x, p->y};
    b.velocity = {p->velX, p->velY};
    b.ownerId = p->ownerId;

    allBullets.emplace(b.id, b);
}

void Client::eventReceiveId(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ServerAssignIdPacket))
        return;

    auto *p = reinterpret_cast<ServerAssignIdPacket *>(event.packet->data);

    allPlayers[p->id].id = p->id;
    Config::Get().playerId = static_cast<int>(p->id);
    ConnexionState = ClientState::CONNECTED;

    std::cout << "[Client] received(ASSIGN_ID): [" << getLocalPlayer(allPlayers)->id << "] \n";
}

void Client::handleTypeConnect(ENetEvent event)
{
    std::cout << "[Client] connected\n";
    peer = event.peer;
    ConnexionState = ClientState::CONNECTING;

    PacketHeader p;
    p.type = static_cast<uint8_t>(PacketType::CLIENT_MSG);

    if (Config::Get().isServer)
        p.code = static_cast<uint8_t>(ClientMsg::REQUEST_NEW_GAME);
    else
        p.code = static_cast<uint8_t>(ClientMsg::REQUEST_JOIN_GAME);

    ENetPacket *packet = enet_packet_create(
        &p,
        sizeof(PacketHeader),
        ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(peer, 0, packet);
}

void Client::eventReceiveInitLevel(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(InitLevelPacket))
        return;

    auto *p = reinterpret_cast<InitLevelPacket *>(event.packet->data);

    terrain.levelSeed = p->seed;
    terrain.worldX = p->worldX;
    terrain.backgroundScrollSpeed = p->scrollSpeed;
    terrain.lookahead = p->lookahead;
    terrain.cleanupMargin = p->cleanupMargin;

    std::cout << "[Client] received INIT_LEVEL: seed=" << terrain.levelSeed
              << " worldX=" << terrain.worldX
              << " scrollSpeed=" << terrain.backgroundScrollSpeed
              << " lookahead=" << terrain.lookahead
              << " cleanupMargin=" << terrain.cleanupMargin
              << "\n";
    // terrain.init(p->seed);
    // terrain.update(terrain.worldX);
}

void Client::eventReceiveWorldX(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(WorldStatePacket))
        return;

    auto *p = reinterpret_cast<WorldStatePacket *>(event.packet->data);

    targetWorldX = p->worldX;
    terrain.worldX = targetWorldX;
    serverGameTime = p->serverGameTime;
    // std::cout << "[Client] received WORLD_X_UPDATE: worldX=" << targetWorldX << "\n";
}

void Client::onReceiveAllSegments(ENetEvent &event)
{
    terrain.segments.clear();

    const uint8_t *ptr = event.packet->data;
    const uint8_t *end = ptr + event.packet->dataLength;

    // Header
    if (ptr + sizeof(ServerAllSegmentsHeader) > end)
        return;

    auto header = reinterpret_cast<const ServerAllSegmentsHeader *>(ptr);
    ptr += sizeof(ServerAllSegmentsHeader);

    for (uint16_t s = 0; s < header->segmentCount; ++s)
    {
        if (ptr + sizeof(ServerSegmentPacket) > end)
            break;

        auto segPkt = reinterpret_cast<const ServerSegmentPacket *>(ptr);
        ptr += sizeof(ServerSegmentPacket);

        TerrainSegment seg;
        seg.type = static_cast<SegmentType>(segPkt->type);
        seg.startX = segPkt->startX;

        for (uint16_t i = 0; i < segPkt->blockCount; ++i)
        {
            if (ptr + sizeof(ServerAllSegmentsBlockPacket) > end)
                break;

            auto blk = reinterpret_cast<const ServerAllSegmentsBlockPacket *>(ptr);
            ptr += sizeof(ServerAllSegmentsBlockPacket);

            seg.blocks.emplace_back(
                TerrainBlock{
                    sf::FloatRect{{blk->x, blk->y}, {blk->w, blk->h}},
                    static_cast<BlockVisual>(blk->visual)});
        }

        terrain.segments.push_back(std::move(seg));
    }
}

void Client::handleTypeReceive(ENetEvent event)
{
    if (event.packet)
    {
        PacketHeader *h = (PacketHeader *)event.packet->data;

        if (h->type == static_cast<uint8_t>(PacketType::SERVER_MSG))
        {
            switch (h->code)
            {
            case static_cast<uint8_t>(ServerMsg::ASSIGN_ID):
                eventReceiveId(event);
                break;
            case static_cast<uint8_t>(ServerMsg::PLAYER_POSITION):
                eventReceivePlayersPositions(event);
                break;
            case static_cast<uint8_t>(ServerMsg::BULLET_SHOOT):
                eventReceiveBullets(event);
                break;
            case static_cast<uint8_t>(ServerMsg::INIT_LEVEL):
                eventReceiveInitLevel(event);
                break;
            case static_cast<uint8_t>(ServerMsg::WORLD_X_UPDATE):
                eventReceiveWorldX(event);
                break;
            case static_cast<uint8_t>(ServerMsg::NEW_SEGMENT):
                onReceiveSegment(event);
                break;
            case static_cast<uint8_t>(ServerMsg::ALL_SEGMENTS):
                onReceiveAllSegments(event);
                break;
            default:
                return;
            }
        }
    }
}

void Client::handleTypeDisconnect(ENetEvent event)
{
    std::cout << "[Client] disconnected\n";
    ConnexionState = ClientState::DISCONNECTED;
    peer = nullptr;
}

void Client::handleEnetService()
{
    ENetEvent event;
    while (enet_host_service(clientHost, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            handleTypeConnect(event);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            handleTypeReceive(event);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            handleTypeDisconnect(event);
            break;

        default:
            break;
        }
    }
}

void Client::update(float dt)
{
    std::lock_guard<std::mutex> lock(mtx);
    handleEnetService();
    sendPosition();
}

void Client::sendPosition()
{
    // envoyer la position au serveur
    if (Config::Get().playerId < 0 || Config::Get().playerId > Config::Get().maxPlayers)
        return;

    if (peer)
    {

        // Clamp pour ne pas sortir de l'écran
        float halfW = Config::Get().playerArea.getCenter().x * Config::Get().playerScale.x;
        float halfH = Config::Get().playerArea.getCenter().y * Config::Get().playerScale.y;

        RemotePlayer *localPlayer = getLocalPlayer(allPlayers);
        if (localPlayer)
        {
            // localPlayer->position.x = std::clamp(
            //     localPlayer->position.x,
            //     halfW,
            //     static_cast<float>(Config::Get().windowSize.x) - halfW);
            // localPlayer->position.y = std::clamp(
            //     localPlayer->position.y,
            //     halfH,
            //     static_cast<float>(Config::Get().windowSize.y) - halfH);

            // localPlayer->position.x = std::clamp(
            //     localPlayer->position.x,
            //     halfW,
            //     Config::Get().windowSize.x - halfW);
            // localPlayer->position.y = std::clamp(
            //     localPlayer->position.y,
            //     halfH,
            //     Config::Get().windowSize.y - halfH);

            ClientPositionPacket p;
            p.header.type = static_cast<uint8_t>(PacketType::CLIENT_MSG);
            p.header.code = static_cast<uint8_t>(ClientMsg::PLAYER_POSITION);
            p.id = localPlayer->id;
            // p.x = localPlayer->position.x;
            // p.y = localPlayer->position.y;
            p.velX = localPlayer->velocity.x;
            p.velY = localPlayer->velocity.y;
            ENetPacket *packet = enet_packet_create(&p, sizeof(p), 0);
            enet_peer_send(peer, 0, packet);

            // std::cout << "[Client] sent POSITION: [" << localPlayer->id << "] x=" << localPlayer->position.x << " y=" << localPlayer->position.y << "\n";
        }
    }
}

void Client::sendBullets()
{
    // envoyer la position au serveur
    if (Config::Get().playerId < 0 || Config::Get().playerId > Config::Get().maxPlayers)
        return;

    if (peer)
    {
        ClientBulletPacket p;
        p.header.type = static_cast<uint8_t>(PacketType::CLIENT_MSG);
        p.header.code = static_cast<uint8_t>(ClientMsg::BULLET_SHOOT);
        p.ownerId = Config::Get().playerId;
        p.x = getLocalPlayer(allPlayers)->getBounds().getCenter().x;
        p.y = getLocalPlayer(allPlayers)->getBounds().getCenter().y;
        p.velX = 1.f;
        p.velY = 0.f;

        ENetPacket *packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE); // Un tir ne doit jamais être perdu.
        enet_peer_send(peer, 0, packet);
    }
}
