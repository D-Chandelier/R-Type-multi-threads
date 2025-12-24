#pragma once
#include <enet/enet.h>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <cmath>

#include "../../Core/Config.hpp"
#include "../../Core/Utils.hpp"

#include "../../Entities/RemotePlayers.hpp"
#include "../../Entities/Bullet.hpp"
#include "../../World/Terrain.hpp"
#include "../Protocols/Packets/Packets.hpp"
#include "../Protocols/NetworkDiscovery.hpp"

#include "../../World/BlockVisual.hpp"

class Server
{
public:
    Server();
    ~Server();

    bool start(uint16_t port);
    void stop();
    void update(float dt);

    void packetBroadcastPositions();                    // envoie positions à tous les clients
    void packetBroadcastBullets(const ServerBullet &b); // envoie les tirs à tous les clients
    void packetBroadcastWorldX();                       // envoie la position monde à tous les clients

    void packetSendAllSegments(ENetPeer *peer);
    void packetSendSegment(const TerrainSegment &seg, ENetPeer *peer);
    void packetSendNewId(ENetEvent event);

    void packetReceivedPlayerPosition(ENetEvent event, float dt);
    void packetReceivedBulletShoot(ENetEvent event);

    void updateTurrets(float dt);

    // TerrainSegment generateNextSegment();

    void handleEnetService(float dt);
    void handleTypeConnect(ENetEvent event);
    void handleTypeReceive(ENetEvent event, float dt);
    void handleTypeDisconnect(ENetEvent event);

    // RemotePlayer *getPlayerByPeer(ENetPeer *peer);

    void checkPlayerCollision(RemotePlayer &player);

    int findFreePlayerId();

    void killAndRespawn(RemotePlayer &p);

    std::atomic_bool serverReady = false;

    uint32_t levelSeed;
    uint64_t levelTick;
    float lookahead;
    float cleanupMargin;
    Terrain terrain;
    float worldX = 0.f;

private:
    ENetHost *host = nullptr;
    std::mutex mtx; // protège players
    std::map<int, RemotePlayer> allPlayers;
    std::unordered_map<uint32_t, ServerBullet> allBullets;
    uint32_t nextBulletId = 1;

    double gameStartTime = 0.0;
    float positionAccumulator = 0.0f;                         // temps écoulé depuis dernier broadcast
    const float SERVER_TICK = 1.0f / Config::Get().frameRate; // 60 Hz
};
