#pragma once
#include "Packets.hpp"

#pragma pack(push, 1)

struct PacketHeader
{
    uint8_t type;
    uint8_t code;
};

struct PlayerState
{
    int id;
    float x;
    float y;
};

struct ServerAssignIdPacket
{
    PacketHeader header;
    uint32_t id;
    double serverStartTime; // secondes (horloge serveur)
};

struct ClientPositionPacket
{
    PacketHeader header;
    int id;
    float x;
    float y;
};

struct ServerPositionPacket
{
    PacketHeader header;
    uint8_t playerCount;
    PlayerState players[MAX_PLAYER];

    double serverGameTime; // temps serveur actuel
    float scrollSpeed;
};

#pragma pack(pop)
