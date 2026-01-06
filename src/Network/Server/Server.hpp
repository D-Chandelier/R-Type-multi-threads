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

#include <random>

#include "../../Core/Config.hpp"
#include "../../Core/Utils.hpp"

#include "../../Entities/RemotePlayers.hpp"
#include "../../Entities/Bullet.hpp"
#include "../../Entities/Enemies.hpp"
#include "../../Entities/Bonus.hpp"
#include "../../Entities/Enemies.hpp"

#include "../Protocols/Packets/Packets.hpp"
#include "../Protocols/NetworkDiscovery.hpp"

#include "../../World/Terrain.hpp"
#include "../../World/BlockVisual.hpp"

class Server
{
public:
    Server();
    ~Server();

    bool start(uint16_t port);
    void stop();
    void update(float dt);

    void packetBroadcastPositions();              // envoie positions à tous les clients
    void packetBroadcastBullets(const Bullet &b); // envoie les tirs à tous les clients
    void packetBroadcastWorldX();                 // envoie la position monde à tous les clients
    void packetBroadcastEnemyDestroyed(uint32_t id);
    void packetBroadcastBulletDestroyed(uint32_t bulletId);
    void packetBroadcastEnemies();
    void packetBroadcastRocket(Bullet &b);
    void packetBroadcastBonuses();
    void packetBroadcastBonusSpawn(const Bonus &b);
    void packetBroadcastRemoveBonus(uint32_t id);
    void packetBroadcastBonusDestroy(uint32_t id);

    void packetSendAllSegments(ENetPeer *peer);
    void packetSendSegment(const TerrainSegment &seg, ENetPeer *peer);
    void packetSendNewId(ENetEvent event);

    void packetReceivedPlayerPosition(ENetEvent event, float dt);
    void packetReceivedBulletShoot(ENetEvent event);
    void packetReceivedRejoin(ENetEvent event);

    void updateSegment();
    void updateEnemies(float dt);
    void updateBonuses(float dt);

    void handleEnetService(float dt);
    void handleTypeConnect(ENetEvent event);
    void handleTypeReceive(ENetEvent event, float dt);
    void handleTypeDisconnect(ENetEvent event);

    void playerCollision(RemotePlayer &player);

    void killAndRespawn(RemotePlayer &p);
    void spawnPlayerMissile(const sf::Vector2f &pos, uint32_t ownerId);
    uint32_t findClosestTarget(sf::Vector2f &from);

    void onEnemyDestroyed(EnemyType enemyType, const sf::Vector2f &pos, RemotePlayer &killer);
    void spawnBonus(BonusType type, sf::Vector2f pos);
    void applyBonus(RemotePlayer &player, Bonus &bonus);
    float randomFloat(float min, float max);

    std::atomic_bool serverReady = false;

    uint32_t levelSeed;
    uint64_t levelTick;
    float lookahead;
    float cleanupMargin;
    Terrain terrain;
    float worldX = 0.f;

    std::unordered_map<uint32_t, Bullet> allBullets;
    std::unordered_map<uint32_t, Enemy> allEnemies;
    std::unordered_map<uint32_t, Bonus> allBonuses;
    std::map<uint32_t, RemotePlayer> allPlayers;

private:
    ENetHost *host = nullptr;
    std::mutex mtx; // protège players
    BonusStats bonusStats;

    uint32_t nextBulletId = 0;
    uint32_t nextEnemyId = 0;
    uint32_t nextBonusId = 0;

    double gameStartTime = 0.0;
    float positionAccumulator = 0.0f;                         // temps écoulé depuis dernier broadcast
    const float SERVER_TICK = 1.0f / Config::Get().frameRate; // 60 Hz
};
