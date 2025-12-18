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

    double serverGameTime;  // temps serveur actuel
    float scrollSpeed; 
};

#pragma pack(pop)
