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
        p.players[i].nbRocket = player.nbRocket;
        p.players[i].fireRate = player.fireRate;

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

void Server::packetBroadcastBullets(const Bullet &b)
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
    p.bulletType = b.type;

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

void Server::packetBroadcastRocket(Bullet &b)
{
    ServerBulletPacket p;
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::ROCKET_STATE);

    p.bulletId = b.id;
    p.x = b.position.x;
    p.y = b.position.y;
    p.velX = b.velocity.x;
    p.velY = b.velocity.y;

    for (auto &[_, player] : allPlayers)
        if (player.peer)
            enet_peer_send(player.peer, 0,
                           enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_UNSEQUENCED));
}

void Server::packetBroadcastWorldX()
{
    if (allPlayers.empty() || !host)
        return;

    WorldStatePacket p;
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::WORLD_X_UPDATE);
    p.worldX = worldX;
    p.serverGameTime = Utils::currentGameTime(gameStartTime);

    for (const auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;
        ENetPacket *packet = enet_packet_create(&p, sizeof(WorldStatePacket), 0);
        enet_peer_send(player.peer, 0, packet);
    }
}

void Server::packetBroadcastEnemyDestroyed(uint32_t id, sf::Vector2f pos)
{
    ServerEnemyDestroyedPacket p{};
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::ENEMY_DESTROYED);
    p.id = id; // envoie l'ID unique de la tourelle
    p.x = pos.x;
    p.y = pos.y;

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

void Server::packetBroadcastEnemies()
{
    ServerEnemiesPacket p{};
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::ENEMIES);
    p.count = std::min<uint8_t>(allEnemies.size(), 128);

    uint8_t i = 0;
    for (auto &[id, enemy] : allEnemies)
    {
        if (i >= p.count)
            break;

        p.enemy[i].id = id;
        p.enemy[i].type = enemy.type;
        p.enemy[i].x = enemy.position.x;
        p.enemy[i].y = enemy.position.y;
        p.enemy[i].velX = enemy.velocity.x;
        p.enemy[i].velY = enemy.velocity.y;
        p.enemy[i].isActive = enemy.active ? 1 : 0;
        ++i;
    }

    for (const auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;

        auto *packet = enet_packet_create(nullptr, sizeof(ServerEnemiesPacket), ENET_PACKET_FLAG_RELIABLE);
        std::memcpy(packet->data, &p, sizeof(ServerEnemiesPacket));

        enet_peer_send(player.peer, 0, packet);
    }
}

void Server::packetBroadcastBonusSpawn(const Bonus &b)
{
    ServerBonusSpawnPacket p{};
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::BONUS_SPAWN);

    p.id = b.id;
    p.type = static_cast<BonusType>(b.type);
    p.x = b.position.x;
    p.y = b.position.y;
    p.sx = b.spawnPos.x;
    p.sy = b.spawnPos.y;
    p.vx = b.velocity.x;
    p.vy = b.velocity.y;
    p.amplitude = b.amplitude;
    p.angularSpeed = b.angularSpeed;
    p.phase = b.phase;

    for (const auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;
        auto *packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(player.peer, 0, packet);
    }
}

void Server::packetBroadcastBonusDestroy(uint32_t id)
{
    ServerBonusDestroyedPacket p{};
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::BONUS_DESTROYED);
    p.id = id;

    for (const auto &[pid, player] : allPlayers)
    {
        if (!player.peer)
            continue;

        auto *packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(player.peer, 0, packet);
    }
}

void Server::packetBroadcastBulletSpawn(Bullet &b)
{
    ServerBulletSpawnPacket p{};
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::BULLET_SPAWN);

    p.id = b.id;
    p.type = static_cast<uint8_t>(b.type);
    p.x = b.position.x;
    p.y = b.position.y;
    p.vx = b.velocity.x;
    p.vy = b.velocity.y;
    p.dammage = b.damage;
    p.owner = static_cast<uint8_t>(b.owner);

    for (const auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;
        auto *packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(player.peer, 0, packet);
    }
};