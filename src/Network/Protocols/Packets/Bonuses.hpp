#pragma once

#include <cstdint>
#include <SFML/Graphics.hpp>
#include "../../../Entities/Bonus.hpp"

#pragma pack(push, 1)

struct BonusState
{
    uint32_t id;
    BonusType type;
    float sx, sy = 0.f;
    float x, y = 0.f;
    float vx, vy = 0.f;
    float time = 0.f;         // temps depuis le spawn
    float amplitude = 0.f;    // hauteur du cercle (ex: 300px)
    float angularSpeed = 0.f; // rad/sec
    float phase = 0.f;        // déphasage

    int active = 0;
};

struct ServerBonusDestroyedPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;
    uint32_t id;
};

struct ServerBonusSpawnPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;

    uint32_t id;
    BonusType type;
    float sx, sy = 0.f;
    float x, y = 0.f;
    float vx, vy = 0.f;
    float time = 0.f;         // temps depuis le spawn
    float amplitude = 0.f;    // hauteur du cercle (ex: 300px)
    float angularSpeed = 0.f; // rad/sec
    float phase = 0.f;        // déphasage

    int active = 0;
};

struct ServerBonusesPacket
{
    struct Header
    {
        uint8_t type;
        uint8_t code;
    } header;
    uint32_t count;
    BonusState bonus[128];
};

#pragma pack(pop)
