#include "Client.hpp"

void Client::packetReceivedInitLevel(ENetEvent event)
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
}

void Client::packetReceivedWorldX(ENetEvent event)
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

void Client::packetReceivedPlayersPositions(ENetEvent event)
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
        if (it->first != Config::Get().playerId && seenIds.find(it->first) == seenIds.end())
            it = allPlayers.erase(it);
        else
            ++it;
    }

    serverTimeOffset = p->serverGameTime - Utils::localTimeNow();
    backgroundScrollSpeed = p->scrollSpeed;
}

void Client::packetReceivedBullets(ENetEvent event)
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

void Client::packetReceivedId(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ServerAssignIdPacket))
        return;

    auto *p = reinterpret_cast<ServerAssignIdPacket *>(event.packet->data);

    allPlayers[p->id].id = p->id;
    Config::Get().playerId = static_cast<int>(p->id);
    ConnexionState = ClientState::CONNECTED;

    std::cout << "[Client] received(ASSIGN_ID): [" << Utils::getLocalPlayer(allPlayers)->id << "] \n";
}
