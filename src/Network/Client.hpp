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

    void updateBackgrounds();
    void updatePlayers();
    void drawPlayers(sf::RenderWindow &w);
    void drawBackground(sf::RenderWindow &w);

    double localTimeNow() const;
    ClientState getState() const { return ConnexionState; }

    ENetHost *clientHost;
    ENetPeer *peer;

    Player localPlayer;
    std::map<int, RemotePlayer> allPlayers;

    sf::Texture backgroundTexture1, backgroundTexture2;

private:
    std::mutex mtx;
    ClientState ConnexionState;

    double serverTimeOffset;
    double backgroundScrollSpeed;
    double background_1_OffsetX = 0.0;
    double background_1_OffsetY = 0.0;
    double background_2_OffsetX = 0.0;
    double background_2_OffsetY = 0.0;
    sf::VertexArray backgroundVA_1, backgroundVA_2, playersVA; // 6 sommets = 2 triangles
};
