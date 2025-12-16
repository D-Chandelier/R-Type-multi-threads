#include "Server.hpp"

Server::Server() : host(nullptr) {}
Server::~Server() { stop(); }
bool Server::start(uint16_t port)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (host)
        return true;

    allPlayers.clear();

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
            int newId = findFreePlayerId();

            ServerAssignIdPacket p;
            p.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
            p.header.code = static_cast<uint8_t>(ServerMsg::ASSIGN_ID);

            if (newId != 100)
            {
                allPlayers[newId].id = newId;
                allPlayers[newId].x = 0.f;
                allPlayers[newId].y = 0.f;
                allPlayers[newId].peer = event.peer;
                // Associer l'id au peer ENet
                // event.peer->data = reinterpret_cast<void *>(static_cast<uintptr_t>(newId));
                // p.id = newId;
                // std::cout << "[SERVER] demande de connexion.\n Envoie ID=" << newId << "\n";
            }
            else
            {
                p.id = 100;
                std::cout << "[SERVER] demande de connexion.\nServer Full (send SPECTATOR ID)\nEnvoie ID=" << newId << " (Server Full)\n";
            }
            // event.peer->data = reinterpret_cast<void *>(static_cast<uintptr_t>(newId));
            p.id = newId;

            ENetPacket *packet = enet_packet_create(
                &p, sizeof(p), ENET_PACKET_FLAG_RELIABLE);

            enet_peer_send(event.peer, 0, packet);
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            if (event.packet)
            {
                PacketHeader *h = (PacketHeader *)event.packet->data;

                if (h->type == static_cast<uint8_t>(PacketType::CLIENT_MSG))
                {
                    if (h->code == static_cast<uint8_t>(ClientMsg::PLAYER_POSITION))
                    {
                        if (event.packet->dataLength != sizeof(ClientPositionPacket))
                            break;
                        auto *p = reinterpret_cast<ClientPositionPacket *>(event.packet->data);
                        auto it = allPlayers.find(p->id);
                        it->second.x = p->x;
                        it->second.y = p->y;

                        allPlayers[p->id].x = std::clamp(it->second.x, 0.f, static_cast<float>(Config::Get().windowSize.x));
                        allPlayers[p->id].y = std::clamp(it->second.y, 0.f, static_cast<float>(Config::Get().windowSize.y));
                    }
                }

                enet_packet_destroy(event.packet);
            }
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "Client déconnecté" << std::endl;

            for (auto it = allPlayers.begin(); it != allPlayers.end(); ++it)
            {
                if (it->second.peer == event.peer)
                {
                    std::cout << "Suppression joueur id=" << it->first << std::endl;

                    allPlayers.erase(it);
                    break;
                }
            }

            event.peer->data = nullptr; // bonne pratique ENet
            break;
        }

        default:
            break;
        }
    }

    // --- 2. Tick serveur pour broadcast des positions ---
    positionAccumulator += dt;
    while (positionAccumulator >= SERVER_TICK)
    {
        broadcastPositions(); // envoi snapshot de toutes les positions
        positionAccumulator -= SERVER_TICK;
    }
}

void Server::broadcastPositions()
{
    if (allPlayers.empty())
        return; // rien à broadcast

    if (!host)
        return;

    ServerPositionPacket snapshot;
    snapshot.header.type = static_cast<uint8_t>(PacketType::SERVER_MSG);
    snapshot.header.code = static_cast<uint8_t>(ServerMsg::PLAYER_POSITION);
    snapshot.playerCount = static_cast<uint8_t>(allPlayers.size());

    int i = 0;
    for (const auto &[id, player] : allPlayers)
    {
        // std::cout << "BROADCAST> " << id << "\n";
        snapshot.players[i].id = player.id;
        snapshot.players[i].x = player.x;
        snapshot.players[i].y = player.y;
        i++;
    }
    ENetPacket *packet = enet_packet_create(&snapshot, sizeof(ServerPositionPacket), 0);

    for (const auto &[id, player] : allPlayers)
        if (player.peer)
            enet_peer_send(player.peer, 0, packet);
    // enet_host_flush(host);
}

std::map<int, RemotePlayer> Server::getPlayers()
{
    std::lock_guard<std::mutex> lock(mtx);
    return allPlayers; // copie thread-safe
}

int Server::findFreePlayerId()
{
    for (uint32_t id = 0; id < Config::Get().maxPlayers; ++id)
    {
        if (allPlayers.find(id) == allPlayers.end())
            return id;
    }
    return 100; // aucun slot libre
}
