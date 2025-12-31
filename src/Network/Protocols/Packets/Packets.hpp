#pragma once
#include <cstdint>
#include <cstddef>
#include <SFML/Graphics.hpp>

#include "Turret.hpp"
#include "Bullet.hpp"
#include "Player.hpp"
#include "Segmant.hpp"

// constexpr std::size_t MAX_PLAYER = 32;

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
    BULLET_SHOOT,
    INIT_LEVEL,
    WORLD_X_UPDATE,
    NEW_SEGMENT,
    ALL_SEGMENTS,
    TURRET,
    TURRET_DESTROYED,
    BULLET_DESTROYED
};

///////////////////////////////////////////////////////:

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
    double serverStartTime; // secondes (horloge serveur)
};

struct ClientRejoinPacket
{
    PacketHeader header;
    uint32_t id;
};

struct InitLevelPacket
{
    PacketHeader header; // code=INIT_LEVEL
    uint32_t seed;       // seed terrain
    float worldX;        // position monde actuelle
    float scrollSpeed;   // vitesse monde
    float lookahead;     // SERVER_LOOKAHEAD
    float cleanupMargin; // SERVER_CLEANUP_MARGIN
};
struct WorldStatePacket
{
    PacketHeader header;   // code=INIT_LEVEL
    uint32_t seed;         // seed terrain
    float worldX;          // position monde actuelle
    double serverGameTime; // temps serveur actuel
};

#pragma pack(pop)
