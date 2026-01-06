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
    std::cout << "Send Enemy destroyed: " << id << "\n";
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

void Server::packetBroadcastBonuses()
{
    // Récupérer les bonus actifs
    std::vector<Bonus *> activeBonuses;
    for (auto &[id, b] : allBonuses)
        activeBonuses.push_back(&b);

    uint32_t count = static_cast<uint32_t>(min(activeBonuses.size(), size_t(128)));

    // Taille buffer : header(2) + count(4) + champs par bonus
    size_t packetSize = 2 + 4 + count * (sizeof(uint32_t) + sizeof(uint16_t) + 10 * sizeof(float) + sizeof(int));
    std::vector<uint8_t> buffer(packetSize, 0);
    uint8_t *ptr = buffer.data();

    // Header
    ptr[0] = static_cast<uint8_t>(PacketType::SERVER_MSG);
    ptr[1] = static_cast<uint8_t>(ServerMsg::BONUS_SPAWN);
    ptr += 2;

    // Count
    std::memcpy(ptr, &count, sizeof(count));
    ptr += sizeof(count);

    // Sérialisation champ par champ
    for (uint32_t i = 0; i < count; ++i)
    {
        Bonus *b = activeBonuses[i];

        std::memcpy(ptr, &b->id, sizeof(b->id));
        ptr += sizeof(b->id);

        uint16_t ttype = static_cast<uint16_t>(b->type);
        std::memcpy(ptr, &ttype, sizeof(ttype));
        ptr += sizeof(ttype);

        float vals[] = {
            b->spawnPos.x, b->spawnPos.y,
            b->position.x, b->position.y,
            b->velocity.x, b->velocity.y,
            b->time, b->amplitude, b->angularSpeed, b->phase};
        std::memcpy(ptr, vals, sizeof(vals));
        ptr += sizeof(vals);

        int tactive = b->active ? 1 : 0;
        std::memcpy(ptr, &tactive, sizeof(tactive));
        ptr += sizeof(tactive);
    }

    // Envoi à tous les joueurs
    for (auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;

        auto *packet = enet_packet_create(buffer.data(), buffer.size(), ENET_PACKET_FLAG_RELIABLE);
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
    p.id = id; // envoie l'ID unique

    std::cout << "[SERVER] send Bonus ID to destroyed: " << p.id << "\n";
    for (const auto &[pid, player] : allPlayers)
    {
        if (!player.peer)
            continue;

        auto *packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(player.peer, 0, packet);
    }
}

// void Server::packetBroadcastBonuses()
// {
//     ServerBonusesPacket p{};
//     p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
//     p.header.code = static_cast<uint8_t>(ServerMsg::BONUSES);
//     p.count = std::min<uint8_t>(allBonuses.size(), 128);

//     uint32_t i = 0;
//     for (auto &[id, bonus] : allBonuses)
//     {
//         if (i >= p.count)
//             break;

//         p.bonus[i].id = id;
//         p.bonus[i].time = bonus.time;
//         p.bonus[i].type = bonus.type;
//         p.bonus[i].sx = bonus.spawnPos.x;
//         p.bonus[i].sy = bonus.spawnPos.y;
//         p.bonus[i].x = bonus.position.x;
//         p.bonus[i].y = bonus.position.y;
//         p.bonus[i].vx = bonus.velocity.x;
//         p.bonus[i].vy = bonus.velocity.y;
//         p.bonus[i].amplitude = bonus.amplitude;
//         p.bonus[i].angularSpeed = bonus.angularSpeed;
//         p.bonus[i].phase = bonus.phase;
//         p.bonus[i].active = bonus.active ? 1 : 0;
//         ++i;
//     }

//     for (const auto &[id, player] : allPlayers)
//     {
//         if (!player.peer)
//             continue;

//         auto *packet = enet_packet_create(nullptr, sizeof(ServerBonusesPacket), ENET_PACKET_FLAG_RELIABLE);
//         std::memcpy(packet->data, &p, sizeof(ServerBonusesPacket));

//         enet_peer_send(player.peer, 0, packet);
//     }
// }