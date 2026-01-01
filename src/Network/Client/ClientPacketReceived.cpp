#include "Client.hpp"

void Client::packetReceivedInitLevel(ENetEvent &event)
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
              << "\n worldX=" << terrain.worldX
              << "\n scrollSpeed=" << terrain.backgroundScrollSpeed
              << "\n lookahead=" << terrain.lookahead
              << "\n cleanupMargin=" << terrain.cleanupMargin
              << "\n";
}

void Client::packetReceivedWorldX(ENetEvent &event)
{
    if (event.packet->dataLength != sizeof(WorldStatePacket))
        return;

    auto *p = reinterpret_cast<WorldStatePacket *>(event.packet->data);

    targetWorldX = p->worldX;
    terrain.worldX = targetWorldX;
    serverGameTime = p->serverGameTime;
}

void Client::packetReceivedAllSegments(ENetEvent &event)
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
                    static_cast<BlockVisual>(blk->visual),
                    blk->hasTurret != 0});
        }

        terrain.segments.push_back(std::move(seg));
    }
}

void Client::packetReceivedSegment(ENetEvent &event)
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
            static_cast<BlockVisual>(b.visual),
            b.hasTurret != 0);
    }

    terrain.segments.push_back(seg);
}

void Client::packetReceivedEnemies(ENetEvent &event)
{
    if (event.packet->dataLength != sizeof(ServerEnemiesPacket))
        return;

    const auto *p = reinterpret_cast<const ServerEnemiesPacket *>(event.packet->data);

    std::unordered_map<uint32_t, Enemy> newEnemies;
    newEnemies.reserve(p->count);

    for (uint8_t i = 0; i < p->count; ++i)
    {
        const auto &s = p->enemy[i];

        Enemy t({s.x, s.y});
        t.type = s.type;
        t.velocity = {s.velX, s.velY};
        t.active = s.isActive == 1;

        newEnemies.emplace(s.id, t);
    }

    {
        std::lock_guard<std::mutex> lock(enemiesMutex);
        allEnemies.swap(newEnemies);
    }
}

void Client::packetReceivedPlayersPositions(ENetEvent &event)
{
    if (event.packet->dataLength != sizeof(ServerPositionPacket))
        return;

    auto *p = reinterpret_cast<ServerPositionPacket *>(event.packet->data);

    std::unordered_set<uint32_t> seenIds;

    for (int i = 0; i < p->playerCount; i++)
    {
        uint32_t pid = p->players[i].id;

        seenIds.insert(pid);

        RemotePlayer &rp = allPlayers[pid];
        rp.id = pid;
        rp.lastUpdateTime = p->serverGameTime;
        rp.serverPosition.x = p->players[i].x;
        rp.serverPosition.y = p->players[i].y;
        rp.alive = p->players[i].alive;
        rp.invulnerable = p->players[i].invulnerable;
        rp.respawnTime = p->players[i].respawnTime;
        rp.score = p->players[i].score;
        rp.pv = p->players[i].pv;
        rp.nbRocket = p->players[i].nbRocket;
    }

    // Supprimer uniquement les joueurs distants qui ne sont plus dans le snapshot
    for (auto it = allPlayers.begin(); it != allPlayers.end();)
    {
        if (it->first != Config::Get().playerId && seenIds.find(it->first) == seenIds.end())
            it = allPlayers.erase(it);
        else
            ++it;
    }

    serverTimeOffset = p->serverGameTime - Utils::localTimeNow();
    backgroundScrollSpeed = p->scrollSpeed;
}

void Client::packetReceivedBullets(ENetEvent &event)
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
    b.type = p->bulletType;

    allBullets.emplace(b.id, b);
}

void Client::packetReceivedRocketState(ENetEvent &event)
{
    auto *p = reinterpret_cast<ServerBulletPacket *>(event.packet->data);

    auto it = allBullets.find(p->bulletId);
    if (it == allBullets.end())
        return;

    Bullet &b = it->second;

    // ⚠️ IMPORTANT
    if (b.type == BulletType::LINEAR)
        return;

    b.position = {p->x, p->y};
    b.velocity = {p->velX, p->velY};
}

void Client::packetReceivedId(ENetEvent &event)
{
    if (event.packet->dataLength != sizeof(ServerAssignIdPacket))
        return;

    auto *p = reinterpret_cast<ServerAssignIdPacket *>(event.packet->data);

    allPlayers[p->id].id = p->id;
    Config::Get().playerId = static_cast<uint32_t>(p->id);
    ConnexionState = ClientState::CONNECTED;

    std::cout << "[Client] received(ASSIGN_ID): [" << Utils::getLocalPlayer(allPlayers)->id << "] \n";
}

void Client::packetReceivedEnemyDestroyed(ENetEvent &event)
{
    if (event.packet->dataLength != sizeof(ServerEnemyDestroyedPacket))
        return;

    auto *p = reinterpret_cast<ServerEnemyDestroyedPacket *>(event.packet->data);

    auto it = allEnemies.find(p->id);
    if (it != allEnemies.end())
    {
        it->second.active = false;
    }
}

void Client::packetReceivedBulletDestroyed(ENetEvent &event)
{
    if (event.packet->dataLength != sizeof(ServerBulletDestroyedPacket))
        return;

    auto *p = reinterpret_cast<ServerBulletDestroyedPacket *>(event.packet->data);
    auto it = allBullets.find(p->bulletIndex);
    if (it != allBullets.end())
    {
        it->second.active = false;
    }
}
