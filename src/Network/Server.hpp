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
    void broadcastPositions(); // envoie positions à tous les clients

    std::map<int, RemotePlayer> getPlayers(); // accès thread-safe
    int findFreePlayerId();

private:
    ENetHost *host = nullptr;
    std::mutex mtx; // protège players
    std::map<int, RemotePlayer> allPlayers;

    float positionAccumulator = 0.0f;                         // temps écoulé depuis dernier broadcast
    const float SERVER_TICK = 1.0f * Config::Get().frameRate; // 60 Hz
};
