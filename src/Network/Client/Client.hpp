#pragma once
#include <SFML/Graphics.hpp>
#include <enet/enet.h>
#include <unordered_set>
#include <iostream>
#include <vector>
#include <mutex>
#include <atomic>

#include "../../Core/Config.hpp"
#include "../../World/Terrain.hpp"

#include "../../Entities/RemotePlayers.hpp"
#include "../../Entities/Enemies.hpp"
#include "../../Entities/Bullet.hpp"
#include "../../Entities/Bonus.hpp"
#include "../../Entities/Animations.hpp"
#include "../Protocols/Packets/Packets.hpp"
#include "../Protocols/NetworkDiscovery.hpp"

#include "../../Core/Utils.hpp"

enum class ClientState
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};

class Client
{
public:
    Client();
    ~Client();

    bool start(const char *host, uint16_t port);
    void update(float dt);
    void stop();
    bool init();

    void handleEnetService();
    void handleTypeConnect(ENetEvent &event);
    void handleTypeReceive(ENetEvent &event);
    void handleTypeDisconnect(ENetEvent &event);

    void packetReceivedId(ENetEvent &event);
    void packetReceivedPlayersPositions(ENetEvent &event);
    void packetReceivedBullets(ENetEvent &event);
    void packetReceivedBulletSpawn(ENetEvent &event);
    void packetReceivedRocketState(ENetEvent &event);
    void packetReceivedInitLevel(ENetEvent &event);
    void packetReceivedWorldX(ENetEvent &event);
    void packetReceivedSegment(ENetEvent &event);
    void packetReceivedAllSegments(ENetEvent &event);
    void packetReceivedEnemyDestroyed(ENetEvent &event);
    void packetReceivedBulletDestroyed(ENetEvent &event);
    void packetReceivedEnemies(ENetEvent &event);
    void packetReceivedBonuses(ENetEvent &event);
    void packetReceivedBonusSpawn(ENetEvent &event);
    void packetReceivedBonusDestroy(ENetEvent &event);

    void packetSendPosition();
    void packetSendBullets(BulletType bType);
    void packedSendRejoin();

    ClientState getState() const { return ConnexionState; }

    ENetHost *clientHost;
    ENetPeer *peer;

    std::map<uint32_t, RemotePlayer> allPlayers;
    std::unordered_map<uint32_t, Enemy> allEnemies, allEnemiesTmp;
    std::unordered_map<uint32_t, Bullet> allBullets;
    std::unordered_map<uint32_t, Bonus> allBonuses;
    std::vector<Explosion> explosions;

    Terrain terrain;
    double serverTimeOffset;
    double backgroundScrollSpeed;
    float targetWorldX;
    float renderWorldX;

    double serverGameTime;

    std::mutex enemiesMutex;
    std::mutex bonusesMutex;

private:
    std::mutex mtx;

    ClientState ConnexionState;
};
