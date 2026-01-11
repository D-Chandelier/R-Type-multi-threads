#pragma once

#include <cstdint>
#include <SFML/Graphics.hpp>
#include "../../../Entities/Bonus.hpp"

#pragma pack(push, 1)

enum class BonusType;

struct BonusState
{
    uint32_t id;
    BonusType type;
    float sx, sy = 0.f;
    float x, y = 0.f;
    float vx, vy = 0.f;
    float time = 0.f;
    float amplitude = 0.f;
    float angularSpeed = 0.f;
    float phase = 0.f;

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
    float time = 0.f;
    float amplitude = 0.f;
    float angularSpeed = 0.f;
    float phase = 0.f;

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
