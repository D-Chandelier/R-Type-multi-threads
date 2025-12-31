#pragma once

#include <cstdint>

#pragma pack(push, 1)

struct ServerAllSegmentsHeader
{
    uint8_t type;
    uint8_t code;
    uint16_t segmentCount;
};

struct ServerSegmentPacket
{

    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;

    uint8_t type; // SegmentType
    float startX;
    uint8_t blockCount;
    struct BlockData
    {
        float x, y, w, h;
        uint8_t visual;
        bool hasTurret;
    } blocks[64];
    uint8_t turretCount;
    struct TurretData
    {
        float x, y;
    } turrets[64];
};

struct ServerAllSegmentsPacket
{
    uint8_t type;
    float startX;
    uint16_t blockCount;
};

struct ServerAllSegmentsBlockPacket
{
    float x, y, w, h;
    uint8_t visual;
    bool hasTurret;
};

#pragma pack(pop)