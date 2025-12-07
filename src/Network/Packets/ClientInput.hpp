#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct ClientInput
{
    uint32_t playerId;
    float dx;
    float dy;
    uint8_t shooting; // 0 or 1
};
#pragma pack(pop)
