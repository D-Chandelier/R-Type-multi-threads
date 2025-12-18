#pragma once
#include <SFML/Graphics.hpp>
#include <enet/enet.h>
#include <unordered_set>
#include <iostream>
#include <vector>
#include <mutex>
#include <atomic>

#include "../Core/Config.hpp"

#include "Packets/Messages.hpp"

#include "../Entities/RemotePlayers.hpp"
#include "../Entities/Player.hpp"
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

    void handleEnetService();
    void handleTypeConnect(ENetEvent event);
    void handleTypeReceive(ENetEvent event);
    void handleTypeDisconnect(ENetEvent event);

    void eventReceiveId(ENetEvent event);
    void eventReceivePlayersPositions(ENetEvent event);

    void sendPosition();

    double localTimeNow() const;
    ClientState getState() const { return ConnexionState; }

    ENetHost *clientHost;
    ENetPeer *peer;

    Player localPlayer;
    std::map<int, RemotePlayer> allPlayers;

    double serverTimeOffset;
    double backgroundScrollSpeed;

private:
    std::mutex mtx;
    ClientState ConnexionState;
};
