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
