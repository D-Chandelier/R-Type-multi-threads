#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct PlayerState
{
    uint32_t id;
    float x;
    float y;
    float vx;
    float vy;
};
#pragma pack(pop)
