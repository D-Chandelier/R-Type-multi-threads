#pragma once
#include <enet/enet.h>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

#include "../Core/Config.hpp"

#include "RemotePlayers.hpp"
#include "NetworkDiscovery.hpp"

class Server
{
public:
    Server();
    ~Server();

    void startDiscovery();
    bool start(uint16_t port);
    void stop();
    void update(float dt);
    void broadcastPositions(); // envoie positions à tous les clients

    std::vector<RemotePlayer> getPlayers(); // accès thread-safe

private:
    ENetHost *host = nullptr;
    std::vector<RemotePlayer> players;
    std::mutex mtx; // protège players

    // contrôle du thread de découverte (local au serveur)
    bool runningDiscovery = false;
    std::thread discoveryThread;
    NetworkDiscovery networkDiscovery;
};
