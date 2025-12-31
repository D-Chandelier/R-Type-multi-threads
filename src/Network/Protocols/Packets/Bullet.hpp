#pragma once

#include <cstdint>
#include <SFML/Graphics.hpp>

#pragma pack(push, 1)

// struct ServerBullet
// {
//     uint32_t id;
//     sf::Vector2f position;
//     sf::Vector2f velocity;
//     float damage;
//     uint32_t ownerId;
//     bool active = true;
// };

struct ClientBulletPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;
    uint32_t ownerId;
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

#pragma pack(pop)
