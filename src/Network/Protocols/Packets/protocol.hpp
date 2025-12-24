#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct NetHeader
{
    uint8_t protocolVersion; // ex: 1
    uint8_t packetType;      // SERVER_MSG / CLIENT_MSG
    uint8_t packetCode;      // ALL_SEGMENTS, SEGMENT, PLAYER_STATE...
    uint16_t payloadSize;    // taille après le header
};
#pragma pack(pop)
