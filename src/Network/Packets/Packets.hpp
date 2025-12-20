#pragma once
#include <cstdint>
#include <cstddef>
#include <SFML/Graphics.hpp>

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
    BULLET_SHOOT
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
    WORLD_X_UPDATE
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
    int id;
    float x;
    float y;
};

struct ServerBullet
{
    uint32_t id;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float damage;
    uint8_t ownerId;
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
    int id;
    float x;
    float y;
};

struct ClientBulletPacket
{
    PacketHeader header;
    int ownerId;
    float x, y;
    float velX, velY;
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
