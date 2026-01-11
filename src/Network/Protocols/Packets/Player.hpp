#pragma once

#include <cstdint>
#include "../../Core/Config.hpp"

#pragma pack(push, 1)

struct PlayerState
{
    uint32_t id;
    float x;
    float y;
    bool alive;
    bool invulnerable;
    double respawnTime;
    float score;
    float pv;
    int nbRocket;
    float fireRate;
};

struct ClientPositionPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;

    uint32_t id;
    float velX;
    float velY;
};

struct ServerPositionPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;

    uint8_t playerCount;
    PlayerState players[static_cast<long>(MAX_PLAYER)];

    double serverGameTime;
    float scrollSpeed;
};

#pragma pack(pop)