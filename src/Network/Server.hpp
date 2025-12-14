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
#include "RemotePlayers.hpp"
#include "NetworkDiscovery.hpp"

class Server
{
public:
    Server();
    ~Server();

    bool start(uint16_t port);
    void stop();
    void update(float dt);
    void onClientMessage(ENetPeer *peer, ClientMsg msg);
    void broadcastPositions(); // envoie positions à tous les clients

    std::vector<RemotePlayer> getPlayers(); // accès thread-safe

private:
    ENetHost *host = nullptr;
    std::mutex mtx; // protège players
    std::vector<RemotePlayer> players;
};
