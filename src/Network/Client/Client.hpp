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
#include "../../Entities/Player.hpp"
#include "../../Entities/Bullet.hpp"
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

    // void reset();
    bool start(const char *host, uint16_t port);
    void update(float dt);
    void stop();
    bool init();

    void handleEnetService();
    void handleTypeConnect(ENetEvent event);
    void handleTypeReceive(ENetEvent event);
    void handleTypeDisconnect(ENetEvent event);

    void packetReceivedId(ENetEvent event);
    void packetReceivedPlayersPositions(ENetEvent event);
    void packetReceivedBullets(ENetEvent event);
    void packetReceivedInitLevel(ENetEvent event);
    void packetReceivedWorldX(ENetEvent event);
    void packetReceivedSegment(ENetEvent &event);
    void packetReceivedAllSegments(ENetEvent &event);

    void packetSendPosition();
    void packetSendBullets();

    ClientState getState() const { return ConnexionState; }

    ENetHost *clientHost;
    ENetPeer *peer;

    // Player localPlayer;
    std::map<int, RemotePlayer> allPlayers;
    std::unordered_map<uint32_t, Bullet> allBullets;

    Terrain terrain;
    double serverTimeOffset;
    double backgroundScrollSpeed;
    float targetWorldX;
    float renderWorldX;

    double serverGameTime;

private:
    std::mutex mtx;
    ClientState ConnexionState;
};
