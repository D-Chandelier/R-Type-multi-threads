#pragma once
#include <enet/enet.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <atomic>
#include "RemotePlayers.hpp"
#include "NetworkDiscovery.hpp"

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

    bool connectTo(const char *host, uint16_t port);
    void update(float dt);

    ClientState ConnexionState = ClientState::DISCONNECTED;
    uint32_t playerId = 0;

    std::vector<RemotePlayer> remotePlayers;
    std::mutex mtx; // protège remotePlayers

private:
    ENetHost *clientHost;
    ENetPeer *peer;
};
