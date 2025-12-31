#include "Server.hpp"

void Server::packetSendAllSegments(ENetPeer *peer)
{
    const auto &segments = terrain.segments;

    size_t packetSize = sizeof(ServerAllSegmentsHeader);

    for (const auto &seg : segments)
        packetSize += sizeof(ServerSegmentPacket) +
                      seg.blocks.size() * sizeof(ServerAllSegmentsBlockPacket);

    ENetPacket *p =
        enet_packet_create(nullptr, packetSize, ENET_PACKET_FLAG_RELIABLE);

    uint8_t *ptr = p->data;

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
            blk.hasTurret = b.hasTurret ? 1 : 0;
            memcpy(ptr, &blk, sizeof(blk));
            ptr += sizeof(blk);
        }

        segPkt.turretCount = static_cast<uint8_t>(seg.turrets.size());
        for (size_t t = 0; t < seg.turrets.size(); ++t)
        {
            segPkt.turrets[t].x = seg.turrets[t].position.x;
            segPkt.turrets[t].y = seg.turrets[t].position.y;
        }
    }

    enet_peer_send(peer, 1, p);
}

void Server::packetSendSegment(const TerrainSegment &seg, ENetPeer *peer)
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
        p.blocks[i].hasTurret = seg.blocks[i].hasTurret ? 1 : 0;
    }
    p.turretCount = static_cast<uint8_t>(seg.turrets.size());
    for (size_t t = 0; t < seg.turrets.size(); ++t)
    {
        p.turrets[t].x = seg.turrets[t].position.x;
        p.turrets[t].y = seg.turrets[t].position.y;
    }
    for (const auto &[id, player] : allPlayers)
    {
        if (!player.peer)
            continue;
        ENetPacket *packet = enet_packet_create(&p, sizeof(p), 0);
        enet_peer_send(peer, 1, packet); // channel 1 = terrain
    }
}

void Server::packetSendNewId(ENetEvent event)
{
    uint32_t newId = Utils::findFreePlayerId(allPlayers);
    ServerAssignIdPacket p;
    p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    p.header.code = static_cast<uint8_t>(ServerMsg::ASSIGN_ID);
    p.serverStartTime = gameStartTime;

    if (newId != 100)
        allPlayers[newId] = RemotePlayer{.id = newId, .position = {Config::Get().playerArea.size.x * Config::Get().playerScale.x * 0.5f, Config::Get().windowSize.y / (MAX_PLAYER + 1.f) * (newId + 1)}, .peer = event.peer};
    p.id = newId;
    event.peer->data = reinterpret_cast<void *>(static_cast<uintptr_t>(newId));

    ENetPacket *packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(event.peer, 0, packet);

    packetSendAllSegments(event.peer);
};