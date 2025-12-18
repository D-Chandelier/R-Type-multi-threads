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

#include "../Core/Config.hpp"

#include "Packets/Messages.hpp"
#include "../Entities/RemotePlayers.hpp"
#include "NetworkDiscovery.hpp"

class Server
{
public:
    Server();
    ~Server();

    bool start(uint16_t port);
    void stop();
    void update(float dt);

    void handleEnetService();
    void handleTypeConnect(ENetEvent event);
    void handleTypeReceive(ENetEvent event);
    void handleTypeDisconnect(ENetEvent event);
    void sendNewId(ENetEvent event);
    void onReceivePlayerPosition(ENetEvent event);

    static double getNowSeconds();
    double currentGameTime() const;
    void broadcastPositions(); // envoie positions à tous les clients

    int findFreePlayerId();
    std::atomic_bool serverReady = false;

private:
    ENetHost *host = nullptr;
    std::mutex mtx; // protège players
    std::map<int, RemotePlayer> allPlayers;

    double gameStartTime = 0.0;
    float positionAccumulator = 0.0f;                         // temps écoulé depuis dernier broadcast
    const float SERVER_TICK = 1.0f / Config::Get().frameRate; // 60 Hz
};
