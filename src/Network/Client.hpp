#pragma once
#include <enet/enet.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <atomic>

#include "../Core/Config.hpp"

#include "Packets/Messages.hpp"
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

    // void reset();
    bool connectTo(const char *host, uint16_t port);
    void update(float dt);
    void stop();
    bool init();
    bool onConnected();
    void sendMsg(ClientMsg msg);

    ClientState getState() const { return ConnexionState; }

private:
    ENetHost *clientHost;
    ENetPeer *peer;
    std::mutex mtx;
    ClientState ConnexionState;
};
