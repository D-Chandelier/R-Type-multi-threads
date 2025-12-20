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

#include "../Core/Config.hpp"

#include "Packets/Packets.hpp"
#include "../Entities/RemotePlayers.hpp"
#include "../Entities/Bullet.hpp"
#include "../World/Terrain.hpp"
#include "NetworkDiscovery.hpp"

class Server
{
public:
    Server();
    ~Server();

    bool start(uint16_t port);
    void stop();
    void update(float dt);
    void sendLevel(ENetPeer *peer);

    void handleEnetService();
    void handleTypeConnect(ENetEvent event);
    void handleTypeReceive(ENetEvent event);
    void handleTypeDisconnect(ENetEvent event);
    void sendNewId(ENetEvent event);
    void onReceivePlayerPosition(ENetEvent event);
    void onReceiveBulletShoot(ENetEvent event);

    static double getNowSeconds();
    double currentGameTime() const;
    RemotePlayer *getPlayerByPeer(ENetPeer *peer);

    void broadcastPositions();                    // envoie positions à tous les clients
    void broadcastWorldX();                       // envoie la position monde à tous les clients
    void broadcastBullets(const ServerBullet &b); // envoie les tirs à tous les clients
    int findFreePlayerId();
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
