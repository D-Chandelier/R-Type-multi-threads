#include "Server.hpp"

Server::Server() : host(nullptr) {}
Server::~Server() { stop(); }

bool Server::start(uint16_t port)
{
    std::lock_guard<std::mutex> lock(mtx);
    players.clear();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    host = enet_host_create(&address, 32, 2, 0, 0);
    if (!host)
        return false;

    std::cout << "[Server] started on port " << port << "\n";
    return true;
}

void Server::stop()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (host)
    {
        enet_host_flush(host);
        enet_host_destroy(host);
        host = nullptr;
    }
}

void Server::update(float dt)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (!host)
        return;

    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            if (players.size() < Config::Get().maxPlayers)
            {
                uint32_t newId = static_cast<uint32_t>(players.size());
                players.push_back({newId, 0.f, 0.f, event.peer});

                std::ostringstream ss;
                ss << "ID:" << newId;
                std::string msg = ss.str();
                ENetPacket *packet = enet_packet_create(msg.c_str(), msg.size() + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(event.peer, 0, packet);
                std::cout << "[Server] Client connected, assigned ID: " << newId << "\n";
            }
            else
            {
                // Serveur plein → joueur spectateur
                std::cout << "[Server] Client connected as spectator\n";
                ENetPacket *packet = enet_packet_create("SPECTATOR", 10, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(event.peer, 0, packet);
            }
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            if (event.packet)
            {
                std::cout << "[Server] received: " << reinterpret_cast<const char *>(event.packet->data) << "\n";

                PacketHeader *h = reinterpret_cast<PacketHeader *>(event.packet->data);

                if (h->type == PacketType::CLIENT_MSG)
                {
                    ClientMsg msg = static_cast<ClientMsg>(h->code);
                    onClientMessage(event.peer, msg);
                }

                enet_packet_destroy(event.packet);
            }
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            // Retirer le joueur si c'était un joueur normal
            auto it = std::find_if(players.begin(), players.end(), [&](const RemotePlayer &p)
                                   { return p.peer == event.peer; });
            if (it != players.end())
            {
                std::cout << "[Server] Client disconnected, ID: " << it->id << "\n";
                players.erase(it);
            }
            else
            {
                std::cout << "[Server] Spectator disconnected\n";
            }
            break;
        }
        default:
            break;
        }
    }
}

void Server::onClientMessage(ENetPeer *peer, ClientMsg msg)
{
    switch (msg)
    {
    case ClientMsg::REQUEST_NEW_GAME:
        std::cout << "[SERVEUR] receive -> REQUEST_NEW_GAME\n";
        // resetGame();
        // addPlayer(peer);
        // send(peer, ServerMsg::GAME_CREATED);
        // broadcast(ServerMsg::START_GAME);
        break;

    case ClientMsg::REQUEST_JOIN_GAME:
        std::cout << "[SERVEUR] receive -> REQUEST_JOIN_GAME\n";

        // addPlayer(peer);
        // send(peer, ServerMsg::GAME_JOINED);
        // if (gameAlreadyStarted)
        //     send(peer, ServerMsg::START_GAME);
        break;
    }
}

void Server::broadcastPositions()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (!host)
        return;

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
