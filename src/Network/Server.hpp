#pragma once
#include <thread>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <enet/enet.h>
#include "ThreadSafeQueue.hpp"
#include "Packets/ClientInput.hpp"
#include "Packets/PlayerState.hpp"

// Structure serveur minimale pour stocker l'état d'un joueur sur le serveur
struct ServerPlayer
{
    uint32_t id;
    float x = 0.f;
    float y = 0.f;
    float vx = 0.f;
    float vy = 0.f;
    float speed = 200.f;
    ENetPeer *peer = nullptr;
};

class Server
{
public:
    Server();
    ~Server();

    bool start(uint16_t port);
    void stop();
    void update(float dt);
    // expose si besoin : push input depuis autre thread (non utilisé ici)
    void pushInput(const ClientInput &in);

private:
    void networkThreadFunc();
    void gameThreadFunc();

    // ENet host
    ENetHost *host = nullptr;

    std::atomic<bool> running{false};
    std::thread networkThread;
    std::thread gameThread;

    ThreadSafeQueue<ClientInput> inputQueue;
    ThreadSafeQueue<std::vector<uint8_t>> outboundPackets; // buffers prêts pour envoi

    // players map (protégé via accès séquentiel entre threads : network thread ajoute peer, game thread lit/écrit positions)
    std::unordered_map<uint32_t, ServerPlayer> players;
    std::mutex playersMutex;

    uint32_t tickCounter = 0;

    // config
    uint32_t maxClients = 32;
    uint16_t listenPort = 1234;
};
