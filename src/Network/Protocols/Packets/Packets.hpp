#pragma once
#include <cstdint>
#include <cstddef>
#include <SFML/Graphics.hpp>

#include "Enemies.hpp"
#include "Bullet.hpp"
#include "Player.hpp"
#include "Segment.hpp"
#include "Bonuses.hpp"

enum class PacketType : uint8_t
{
    CLIENT_MSG,
    SERVER_MSG
};

enum class ClientMsg : uint8_t
{
    REQUEST_NEW_GAME,
    REQUEST_JOIN_GAME,
    PLAYER_POSITION,
    BULLET_SHOOT,
    REJOIN
};

enum class ServerMsg : uint8_t
{
    ASSIGN_ID,
    GAME_CREATED,
    GAME_JOINED,
    START_GAME,
    PLAYER_POSITION,
    BACKGROUND_STATE,
    INIT_LEVEL,
    BULLET_SHOOT,
    BULLET_SPAWN,
    BULLET_DESTROYED,
    ROCKET_STATE,
    WORLD_X_UPDATE,
    NEW_SEGMENT,
    ALL_SEGMENTS,
    ENEMIES,
    ENEMY_DESTROYED,
    BONUS_SPAWN,
    BONUS_DESTROYED
};

#pragma pack(push, 1)

struct PacketHeader
{
    uint8_t type;
    uint8_t code;
};

struct ServerAssignIdPacket
{
    PacketHeader header;
    uint32_t id;
    double serverStartTime;
};

struct ClientRejoinPacket
{
    PacketHeader header;
    uint32_t id;
};

struct InitLevelPacket
{
    PacketHeader header;
    uint32_t seed;
    float worldX;
    float scrollSpeed;
    float lookahead;
    float cleanupMargin;
};
struct WorldStatePacket
{
    PacketHeader header;
    uint32_t seed;
    float worldX;
    double serverGameTime;
};

#pragma pack(pop)
