#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include "Packets/Snapshot.hpp"
#include "Packets/PlayerState.hpp"

// Sérialisation simple pour Snapshot:
// buffer = [SnapshotHeader][PlayerState 0][PlayerState 1]...
inline std::vector<uint8_t> serializeSnapshot(uint32_t tick, const std::vector<PlayerState> &players)
{
    SnapshotHeader h;
    h.type = 2;
    h.tick = tick;
    h.playerCount = static_cast<uint32_t>(players.size());

    size_t total = sizeof(SnapshotHeader) + players.size() * sizeof(PlayerState);
    std::vector<uint8_t> buf;
    buf.resize(total);

    size_t offset = 0;
    std::memcpy(buf.data() + offset, &h, sizeof(SnapshotHeader));
    offset += sizeof(SnapshotHeader);

    if (!players.empty())
    {
        std::memcpy(buf.data() + offset, players.data(), players.size() * sizeof(PlayerState));
    }
    return buf;
}

// Désérialisation : remplir header et vecteur players depuis un buffer reçu (sûr si taille cohérente)
inline bool deserializeSnapshot(const uint8_t *data, size_t len, SnapshotHeader &outHeader, std::vector<PlayerState> &outPlayers)
{
    if (len < sizeof(SnapshotHeader))
        return false;
    std::memcpy(&outHeader, data, sizeof(SnapshotHeader));
    size_t expected = sizeof(SnapshotHeader) + (size_t)outHeader.playerCount * sizeof(PlayerState);
    if (len < expected)
        return false;
    outPlayers.resize(outHeader.playerCount);
    if (outHeader.playerCount > 0)
    {
        std::memcpy(outPlayers.data(), data + sizeof(SnapshotHeader), outHeader.playerCount * sizeof(PlayerState));
    }
    return true;
}
