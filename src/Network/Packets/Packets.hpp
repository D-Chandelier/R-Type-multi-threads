#pragma once
#include <cstdint>

constexpr std::size_t MAX_PLAYER = 32;

enum class PacketType : uint8_t
{
    CLIENT_MSG,
    SERVER_MSG
};

enum class ClientMsg : uint8_t
{
    REQUEST_NEW_GAME,
    REQUEST_JOIN_GAME,
    PLAYER_POSITION
};

enum class ServerMsg : uint8_t
{
    ASSIGN_ID,
    GAME_CREATED,
    GAME_JOINED,
    START_GAME,
    PLAYER_POSITION,
    BACKGROUND_STATE
};
