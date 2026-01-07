#pragma once

#include <cstdint>
#include <SFML/Graphics.hpp>
#include "../../../Entities/Bullet.hpp"

#pragma pack(push, 1)

struct ClientBulletPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;
    uint32_t ownerId;
    BulletType bulletType;
    float x, y;
    float velX, velY;
};

struct ServerBulletPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;
    uint32_t bulletId;
    BulletType bulletType;
    float x, y;
    float velX, velY;
    uint8_t ownerId;
};

struct bulletPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;
    uint32_t bulletId;
    float x, y;
    float velX, velY;
    uint8_t ownerId;
};

struct ServerBulletDestroyedPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;
    uint16_t bulletIndex;
};

struct ServerBulletSpawnPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;
    uint32_t id;
    uint8_t type;
    float x, y;
    float vx, vy;
    float dammage;
    uint8_t owner;
};

#pragma pack(pop)
