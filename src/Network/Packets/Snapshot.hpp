#pragma once
#include <cstdint>
#include <vector>
#include "PlayerState.hpp"

#pragma pack(push, 1)
struct SnapshotHeader
{
    uint8_t type; // par ex. 2 = snapshot
    uint32_t tick;
    uint32_t playerCount;
    // suivi des PlayerState (playerCount * sizeof(PlayerState))
};
#pragma pack(pop)
