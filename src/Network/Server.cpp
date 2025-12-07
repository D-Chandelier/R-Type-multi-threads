#include "Server.hpp"
#include <sstream>

Server::Server()
    : host(nullptr)
{
    // On initialise ENet ici pour le serveur.
    // NOTE: si tu as aussi un Client qui dépend d'ENet, évite d'appeler enet_deinitialize()
    // dans le destructeur du Server — on le gèrera globalement ailleurs si nécessaire.
    if (enet_initialize() != 0)
    {
        std::cerr << "[Server] ENet init failed\n";
    }
}

Server::~Server()
{
    // Stop discovery thread proprement
    runningDiscovery = false;
    if (discoveryThread.joinable())
        discoveryThread.join();

    // Détruire l'host ENet
    if (host)
        enet_host_destroy(host);

    // Ne pas appeler enet_deinitialize() ici si d'autres composants utilisent ENet.
}

void Server::startDiscovery()
{
    // Lance le thread membre discoveryThread (clé : ne PAS créer une variable locale)
    runningDiscovery = true;
    discoveryThread = std::thread([this]()
                                  {
        while (this->runningDiscovery) {
            // envoie le port du serveur via NetworkDiscovery (SFML UDP)
            this->networkDiscovery.broadcastServer(Config::Get().serverPort, Config::Get().discoveryPort);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        } });
}

bool Server::start(uint16_t port)
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    host = enet_host_create(&address, 32, 2, 0, 0);
    if (!host)
    {
        std::cerr << "[Server] Failed to create ENet server host\n";
        return false;
    }

    std::cout << "[Server] started on port " << port << "\n";
    // this->gamePort = port;

    // Lancer la découverte réseau (thread membre)
    startDiscovery();

    return true;
}

void Server::stop()
{
    // méthode publique pour arrêter le serveur proprement
    runningDiscovery = false;
    if (discoveryThread.joinable())
        discoveryThread.join();

    if (host)
    {
        enet_host_flush(host);
        enet_host_destroy(host);
        host = nullptr;
    }
}

void Server::update(float dt)
{
    if (!host)
        return;

    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            std::cout << "[Server] client connected\n";
            std::lock_guard<std::mutex> lock(mtx);
            players.push_back({static_cast<uint32_t>(players.size()), 0.f, 0.f});
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            const char *msg = reinterpret_cast<const char *>(event.packet->data);
            if (msg && strcmp(msg, "PING") == 0)
            {
                std::cout << "[Server] received PING\n";
                ENetPacket *reply = enet_packet_create("PONG", 5, ENET_PACKET_FLAG_UNSEQUENCED);
                enet_peer_send(event.peer, 0, reply);
            }
            else if (msg)
            {
                std::cout << "[Server] received: " << msg << "\n";
            }
            enet_packet_destroy(event.packet);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "[Server] client disconnected\n";
            std::lock_guard<std::mutex> lock(mtx);
            if (!players.empty())
                players.pop_back();
            break;
        }
        default:
            break;
        }
    }
}

void Server::broadcastPositions()
{
    if (!host)
        return;

    std::lock_guard<std::mutex> lock(mtx);

    // pour chaque player on envoie sa position à chaque peer connecté
    for (auto &player : players)
    {
        std::ostringstream ss;
        ss << player.id << ":" << player.x << "," << player.y;
        std::string msg = ss.str();

        // ENet prend possession du paquet, donc on crée un paquet par envoi
        for (size_t i = 0; i < host->peerCount; ++i)
        {
            ENetPeer *peer = &host->peers[i];
            if (peer->state == ENET_PEER_STATE_CONNECTED)
            {
                ENetPacket *packet = enet_packet_create(msg.c_str(), msg.size() + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(peer, 0, packet);
            }
        }
    }

    // s'assurer que les paquets sont effectivement envoyés
    enet_host_flush(host);
}

std::vector<RemotePlayer> Server::getPlayers()
{
    std::lock_guard<std::mutex> lock(mtx);
    return players; // copie thread-safe
}
