#pragma once

#include "Packets.hpp"

#pragma pack(push, 1)

struct EnemyState
{
    uint32_t id;
    EnemyType type;
    float x, y;
    float velX, velY;
    int isActive;
};

struct ServerEnemiesPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;
    uint32_t count;
    EnemyState enemy[128];
};

struct ServerEnemyDestroyedPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;
    uint32_t id;
};
#pragma pack(pop)