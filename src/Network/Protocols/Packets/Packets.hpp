#pragma once
#include <cstdint>
#include <cstddef>
#include <SFML/Graphics.hpp>

#include "Turret.hpp"

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
};

struct ServerBullet
{
    uint32_t id;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float damage;
    uint32_t ownerId;
    bool active = true;
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
    uint32_t id;
    // float x;
    // float y;
    float velX;
    float velY;
};

struct ClientBulletPacket
{
    PacketHeader header;
    uint32_t ownerId;
    float x, y;
    float velX, velY;
};

struct ClientRejoinPacket
{
    PacketHeader header;
    uint32_t id;
};

struct ServerPositionPacket
{
    PacketHeader header;
    uint8_t playerCount;
    PlayerState players[MAX_PLAYER];

    double serverGameTime; // temps serveur actuel
    float scrollSpeed;
};
struct ServerBulletPacket
{
    PacketHeader header;
    uint32_t bulletId;
    float x, y;
    float velX, velY;
    uint8_t ownerId;
};

struct ServerBulletDestroyedPacket
{
    PacketHeader header;
    uint16_t bulletIndex;
};

struct ServerTurretDestroyedPacket
{
    PacketHeader header;
    uint16_t turretIndex;
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

struct ServerSegmentPacket
{

    PacketHeader header;
    uint8_t type; // SegmentType
    float startX;
    uint8_t blockCount;
    struct BlockData
    {
        float x, y, w, h;
        uint8_t visual;
        bool hasTurret;
    } blocks[64];
    uint8_t turretCount;
    struct TurretData
    {
        float x, y;
    } turrets[128];
};

// struct ServerAllSegmentsPacket
// {

//     PacketHeader header;
//     uint8_t segmentCount;
//     struct SegmentData
//     {
//         uint8_t type; // SegmentType
//         float startX;
//         uint8_t blockCount;
//         struct BlockData
//         {
//             float x, y, w, h;
//         } blocks[64]; // max 8 blocs par segment
//     } segments[32];   // max 32 segments
// };

struct ServerAllSegmentsHeader
{
    uint8_t type;
    uint8_t code;
    uint16_t segmentCount;
};

struct ServerAllSegmentsPacket
{
    uint8_t type;
    float startX;
    uint16_t blockCount;
};
struct ServerAllSegmentsBlockPacket
{
    float x, y, w, h;
    uint8_t visual;
    bool hasTurret;
};

// struct ServerTurretPacket
// {
//     float x, y;
// };

#pragma pack(pop)
