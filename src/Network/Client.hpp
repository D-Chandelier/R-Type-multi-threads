#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <enet/enet.h>
#include "ThreadSafeQueue.hpp"
#include "Packets/ClientInput.hpp"
#include "Packets/PlayerState.hpp"
#include "Packets/Snapshot.hpp"
#include "../Gameplay/RemotePlayers.hpp"

// Callback type for snapshot received
using SnapshotCallback = void (*)(uint32_t tick, const std::vector<PlayerState> &players);

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

    void update(float dt);
    ClientState ConnexionState = ClientState::DISCONNECTED;

    bool start();
    void stop();

    bool connectTo(const char *host, uint16_t port);
    void disconnect();

    // envoi d'un input au serveur (non bloquant)
    void sendInput(const ClientInput &in);

    // registre callback pour snapshots
    void setSnapshotCallback(SnapshotCallback cb);

    uint32_t playerId;
    std::map<uint32_t, RemotePlayer> remotePlayers;

private:
    void networkThreadFunc();

    ENetHost *clientHost = nullptr;
    ENetPeer *serverPeer = nullptr;
    std::thread netThread;
    std::atomic<bool> running{false};

    ThreadSafeQueue<ClientInput> outInputs;
    SnapshotCallback snapshotCb = nullptr;
};
