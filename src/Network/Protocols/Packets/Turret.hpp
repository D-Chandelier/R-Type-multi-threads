#pragma once
#include <cstdint>

#pragma pack(push, 1)

struct TurretState
{
    uint32_t id;
    float x, y;
    float velX, velY;
    bool isActive;
};

struct ServerTurretsPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;
    uint8_t turretCount;
    TurretState turret[32];
};

#pragma pack(pop)