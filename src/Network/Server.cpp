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
    terrain.update(worldX);

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
        p.players[i].x = player.x;
        p.players[i].y = player.y;
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

void Server::sendNewId(ENetEvent event)
{
    int newId = findFreePlayerId();
    ServerAssignIdPacket p;
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::ASSIGN_ID);
    p.serverStartTime = gameStartTime;

    if (newId != 100)
        allPlayers[newId] = RemotePlayer{.id = newId, .x = 0.f, .y = 0.f, .peer = event.peer};
    else
        p.id = 100;

    p.id = newId;
    event.peer->data = reinterpret_cast<void *>(static_cast<uintptr_t>(newId));

    ENetPacket *packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(event.peer, 0, packet);

    sendLevel(event.peer);
};

void Server::onReceivePlayerPosition(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ClientPositionPacket))
        return;
    auto *p = reinterpret_cast<ClientPositionPacket *>(event.packet->data);
    auto it = allPlayers.find(p->id);
    if (it == allPlayers.end())
        return;
    it->second.x = std::clamp(p->x, 0.f, static_cast<float>(Config::Get().windowSize.x));
    it->second.y = std::clamp(p->y, 0.f, static_cast<float>(Config::Get().windowSize.y));
}

void Server::onReceiveBulletShoot(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ClientBulletPacket))
        return;

    auto *p = reinterpret_cast<ClientBulletPacket *>(event.packet->data);

    RemotePlayer *shooter = getPlayerByPeer(event.peer);
    if (!shooter)
        return;
    // if (!canShoot(shooter))
    //     return;

    sf::Vector2f dir{p->velX, p->velY};
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0.f)
        return;

    dir /= len;

    ServerBullet bullet;
    bullet.id = nextBulletId++;
    bullet.position = sf::Vector2f(shooter->x, shooter->y); // sf::Vector2f{shooter->serverX, shooter->serverY}; // PAS p->x/p->y
    bullet.velocity = dir * shooter->bulletSpeed;
    ;
    bullet.damage = shooter->bulletDamage;
    bullet.ownerId = shooter->id;

    allBullets.emplace(bullet.id, bullet);
    broadcastBullets(bullet);
}

void Server::handleTypeConnect(ENetEvent event)
{
    sendNewId(event);
}

void Server::handleTypeReceive(ENetEvent event)
{
    if (!event.packet)
        return;

    PacketHeader *h = (PacketHeader *)event.packet->data;
    if (h->type != static_cast<uint8_t>(PacketType::CLIENT_MSG))
        return;
    switch (h->code)
    {
    case static_cast<uint8_t>(ClientMsg::PLAYER_POSITION):
        onReceivePlayerPosition(event);
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

void Server::handleEnetService()
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
            handleTypeReceive(event);
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

    handleEnetService();

    positionAccumulator += dt;

    while (positionAccumulator >= SERVER_TICK)
    {
        positionAccumulator -= SERVER_TICK;
        levelTick++;
        worldX += LEVEL_SCROLL_SPEED * SERVER_TICK;

        terrain.update(worldX);
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
