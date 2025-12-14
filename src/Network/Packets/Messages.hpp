#pragma once
#include <cstdint>

enum class PacketType : uint8_t
{
    CLIENT_MSG,
    SERVER_MSG
};

enum class ClientMsg : uint8_t
{
    REQUEST_NEW_GAME,
    REQUEST_JOIN_GAME
};

enum class ServerMsg : uint8_t
{
    GAME_CREATED,
    GAME_JOINED,
    START_GAME
};

#pragma pack(push, 1)
struct PacketHeader
{
    PacketType type;
    uint8_t code;
};
#pragma pack(pop)
