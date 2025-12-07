#include "Server.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include "NetworkHelpers.hpp"

Server::Server() {}
Server::~Server() { stop(); }

bool Server::start(uint16_t port)
{
    listenPort = port;
    if (enet_initialize() != 0)
    {
        std::cerr << "[Server] enet_initialize failed\n";
        return false;
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    host = enet_host_create(&address, maxClients, 2, 0, 0);
    if (!host)
    {
        std::cerr << "[Server] enet_host_create failed\n";
        enet_deinitialize();
        return false;
    }

    running = true;
    networkThread = std::thread(&Server::networkThreadFunc, this);
    gameThread = std::thread(&Server::gameThreadFunc, this);
    std::cout << "[Server] started on port " << port << "\n";
    return true;
}

void Server::stop()
{
    bool wasRunning = running.exchange(false);
    if (!wasRunning)
        return;

    if (networkThread.joinable())
        networkThread.join();
    if (gameThread.joinable())
        gameThread.join();

    if (host)
    {
        enet_host_flush(host);
        enet_host_destroy(host);
        host = nullptr;
    }
    enet_deinitialize();
    std::cout << "[Server] stopped\n";
}

void Server::pushInput(const ClientInput &in)
{
    inputQueue.push(in);
}

/////////////////////////
// THREAD RESEAU
/////////////////////////
void Server::networkThreadFunc()
{
    ENetEvent event;
    while (running)
    {
        // timeout 5ms -> boucle réactive
        while (enet_host_service(host, &event, 5) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
            {
                std::cout << "[Server][Net] connect\n";
                // créer un ServerPlayer pour ce peer
                {
                    std::lock_guard<std::mutex> lk(playersMutex);
                    ServerPlayer sp;
                    // on peut générer un id unique (par ex. peer->incomingPeerID)
                    uint32_t id = static_cast<uint32_t>(event.peer->incomingPeerID);
                    sp.id = id;
                    sp.peer = event.peer;
                    players[id] = sp;
                }
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                // attente : le client envoie des ClientInput structs
                if (event.packet && event.packet->dataLength >= sizeof(ClientInput))
                {
                    ClientInput in;
                    std::memcpy(&in, event.packet->data, sizeof(ClientInput));
                    inputQueue.push(in);
                }
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                std::cout << "[Server][Net] disconnect\n";
                // remove player
                std::lock_guard<std::mutex> lk(playersMutex);
                for (auto it = players.begin(); it != players.end(); ++it)
                {
                    if (it->second.peer == event.peer)
                    {
                        players.erase(it);
                        break;
                    }
                }
                break;
            }
            default:
                break;
            }
        }

        // envoyer les paquets produits par gameThread
        std::vector<uint8_t> buf;
        while (outboundPackets.try_pop(buf))
        {
            ENetPacket *packet = enet_packet_create(buf.data(), buf.size(), ENET_PACKET_FLAG_UNSEQUENCED);
            enet_host_broadcast(host, 0, packet);
            enet_host_flush(host);
        }
    }
}

/////////////////////////
// THREAD JEU (60Hz)
/////////////////////////
void Server::gameThreadFunc()
{
    const float TICK_RATE = 1.f / 60.f;
    using clk = std::chrono::high_resolution_clock;

    while (running)
    {
        auto frameStart = clk::now();

        // 1) Consommer tous les inputs en attente
        ClientInput cinp;
        while (inputQueue.try_pop(cinp))
        {
            // Appliquer input : trouver player et modifier vx/vy
            std::lock_guard<std::mutex> lk(playersMutex);
            auto it = players.find(cinp.playerId);
            if (it != players.end())
            {
                it->second.vx = cinp.dx * it->second.speed;
                it->second.vy = cinp.dy * it->second.speed;
                // handle shooting flag si nécessaire
            }
        }

        // 2) Mettre à jour la simulation (players, bullets, ennemis...)
        {
            std::lock_guard<std::mutex> lk(playersMutex);
            for (auto &kv : players)
            {
                ServerPlayer &p = kv.second;
                p.x += p.vx * TICK_RATE;
                p.y += p.vy * TICK_RATE;
                // collisions, limites, etc. -> à implémenter ici
            }
        }

        // update bullets / enemymgr etc. -> appeler tes managers avec dt fixe
        // bulletManager.update(TICK_RATE);
        // enemyManager.update(TICK_RATE);

        // 3) Construire snapshot
        std::vector<PlayerState> states;
        {
            std::lock_guard<std::mutex> lk(playersMutex);
            states.reserve(players.size());
            for (auto &kv : players)
            {
                PlayerState s;
                s.id = kv.second.id;
                s.x = kv.second.x;
                s.y = kv.second.y;
                s.vx = kv.second.vx;
                s.vy = kv.second.vy;
                states.push_back(s);
            }
        }

        auto buf = serializeSnapshot(tickCounter++, states);
        outboundPackets.push(buf);

        // 4) sleep pour atteindre 60Hz
        auto frameEnd = clk::now();
        std::chrono::duration<float> elapsed = frameEnd - frameStart;
        float wait = TICK_RATE - elapsed.count();
        if (wait > 0.f)
        {
            std::this_thread::sleep_for(std::chrono::duration<float>(wait));
        }
    }
}
