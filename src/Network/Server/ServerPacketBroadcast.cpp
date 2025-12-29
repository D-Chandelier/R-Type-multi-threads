#include "Server.hpp"

void Server::packetBroadcastPositions()
{
    if (allPlayers.empty() || !host)
        return; // rien à broadcast

    ServerPositionPacket p;
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::PLAYER_POSITION);
    p.playerCount = static_cast<uint8_t>(allPlayers.size());

    p.serverGameTime = Utils::currentGameTime(gameStartTime);
    p.scrollSpeed = BACKGROUND_SCROLL_SPEED;

    uint32_t i = 0;
    for (const auto &[id, player] : allPlayers)
    {
        p.players[i].id = player.id;
        p.players[i].x = player.position.x;
        p.players[i].y = player.position.y;
        p.players[i].alive = player.alive;
        p.players[i].invulnerable = player.invulnerable;
        p.players[i].respawnTime = player.respawnTime;
        p.players[i].score = player.score;
        p.players[i].pv = player.pv;

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

void Server::packetBroadcastBullets(const ServerBullet &b)
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

void Server::packetBroadcastWorldX()
{
    if (allPlayers.empty() || !host)
        return; // rien à broadcast

    WorldStatePacket p;
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::WORLD_X_UPDATE); // Assurez-vous que ce code existe dans votre enum ServerMsg
    p.worldX = worldX;
    p.serverGameTime = Utils::currentGameTime(gameStartTime);

    // Envoi du paquet à tous les joueurs connectés
    for (const auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;
        ENetPacket *packet = enet_packet_create(&p, sizeof(WorldStatePacket), 0);
        enet_peer_send(player.peer, 0, packet);
    }
}

void Server::packetBroadcastTurretDestroyed(uint32_t turretId)
{
    ServerTurretDestroyedPacket p{};
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::TURRET_DESTROYED);
    p.turretIndex = turretId; // envoie l'ID unique de la tourelle

    for (const auto &[id, player] : allPlayers)
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

void Server::packetBroadcastBulletDestroyed(uint32_t bulletId)
{
    ServerBulletDestroyedPacket p{};
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::BULLET_DESTROYED);
    p.bulletIndex = bulletId; // envoie l'ID unique de la tourelle

    for (const auto &[id, player] : allPlayers)
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

void Server::packetBroadcastTurrets()
{
    ServerTurretsPacket p{};
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::TURRET);
    p.turretCount = std::min<uint8_t>(allTurrets.size(), 32);

    uint8_t i = 0;
    for (auto &[id, turret] : allTurrets)
    {
        if (i >= p.turretCount)
            break;

        p.turret[i].id = id;
        p.turret[i].x = turret.position.x;
        p.turret[i].y = turret.position.y;
        p.turret[i].velX = turret.velocity.x;
        p.turret[i].velY = turret.velocity.y;
        p.turret[i].isActive = turret.active;
        ++i;
    }

    for (const auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;

        auto *packet = enet_packet_create(nullptr, sizeof(ServerTurretsPacket), ENET_PACKET_FLAG_RELIABLE);
        std::memcpy(packet->data, &p, sizeof(ServerTurretsPacket));

        enet_peer_send(player.peer, 0, packet);
    }
}