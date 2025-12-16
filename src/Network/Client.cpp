#include "Client.hpp"
#include <unordered_set>

Client::Client()
    : clientHost(nullptr),
      peer(nullptr),
      ConnexionState(ClientState::DISCONNECTED)
{
}

Client::~Client() { stop(); }

bool Client::init()
{
    if (clientHost)
        return true; // déjà créé
    clientHost = enet_host_create(nullptr, 1, 2, 0, 0);
    return clientHost != nullptr;
}

bool Client::connectTo(const char *host, uint16_t port)
{
    init();
    if (!clientHost)
        return false;

    ENetAddress address;
    enet_address_set_host(&address, host);
    address.port = port;

    peer = enet_host_connect(clientHost, &address, 2, 0);
    if (!peer)
        return false;

    ConnexionState = ClientState::CONNECTING;
    std::cout << "[Client] connecting to " << host << ":" << port << "\n";
    return true;
}

void Client::stop()
{
    if (peer)
    {
        enet_host_flush(clientHost);
        enet_peer_disconnect(peer, 0);
        peer = nullptr;
    }

    if (clientHost)
    {
        enet_host_flush(clientHost);
        enet_host_destroy(clientHost);
        clientHost = nullptr;
    }
}

void Client::update(float dt)
{
    std::lock_guard<std::mutex> lock(mtx);

    if (!clientHost)
        return;

    ENetEvent event;
    while (enet_host_service(clientHost, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            std::cout << "[Client] connected\n";
            peer = event.peer;
            ConnexionState = ClientState::CONNECTING;

            if (Config::Get().isServer)
                sendMsg(ClientMsg::REQUEST_NEW_GAME);
            else
                sendMsg(ClientMsg::REQUEST_JOIN_GAME);
        };
        break;
        case ENET_EVENT_TYPE_RECEIVE:

            if (event.packet)
            {
                PacketHeader *h = (PacketHeader *)event.packet->data;

                if (h->type == static_cast<uint8_t>(PacketType::SERVER_MSG))
                {
                    if (h->code == static_cast<uint8_t>(ServerMsg::ASSIGN_ID))
                    {
                        if (event.packet->dataLength != sizeof(ServerAssignIdPacket))
                            break;
                        auto *p = reinterpret_cast<ServerAssignIdPacket *>(event.packet->data);
                        allPlayers[p->id].id = p->id;
                        localPlayer.id = p->id;
                        localPlayer.position = {0.f, 0.f};
                        std::cout << "[Client] received(ASSIGN_ID): [" << localPlayer.id << "] \n";
                        ConnexionState = ClientState::CONNECTED;
                    }

                    if (h->code == static_cast<uint8_t>(ServerMsg::PLAYER_POSITION))
                    {
                        // std::cout << "[Client received] ServerPositionPacket\n";
                        if (event.packet->dataLength != sizeof(ServerPositionPacket))
                            break;

                        auto *p = reinterpret_cast<ServerPositionPacket *>(event.packet->data);

                        std::unordered_set<int> seenIds;
                        for (int i = 0; i < p->playerCount; i++)
                        {
                            seenIds.insert(p->players[i].id);
                            allPlayers[p->players[i].id].id = p->players[i].id;
                            allPlayers[p->players[i].id].x = p->players[i].x;
                            allPlayers[p->players[i].id].y = p->players[i].y;
                        }

                        // Supprimer les joueurs qui ne sont plus dans le snapshot
                        for (auto it = allPlayers.begin(); it != allPlayers.end();)
                        {
                            if (seenIds.find(it->first) == seenIds.end())
                                it = allPlayers.erase(it);
                            else
                                ++it;
                        }

                        // allPlayers.emplace(
                        //     i,
                        //     RemotePlayer{
                        //         p->players[i].id,
                        //         p->players[i].x,
                        //         p->players[i].y,
                        //         nullptr});

                        // if (p->players[i].id == localPlayer.id)
                        // {
                        //     localPlayer.position.x = static_cast<float>(p->players[i].x);
                        //     localPlayer.position.y = static_cast<float>(p->players[i].y);
                        // }
                        // else
                        // {

                        // auto it = allPlayers.find(p->players[i].id);
                        // if (it == allPlayers.end())
                        // {
                        //     RemotePlayer rp;
                        //     rp.id = p->players[i].id;
                        //     rp.x = p->players[i].x;
                        //     rp.y = p->players[i].y;
                        //     rp.peer = nullptr; // ou peer connu si c’est le connect
                        //     allPlayers[p->players[i].id] = rp;
                        // }
                        // else
                        // {
                        //     it->second.id = p->players[i].id;
                        //     it->second.x = p->players[i].x;
                        //     it->second.y = p->players[i].y;
                        // }
                        // allPlayers[p->id].x = p->x;
                        // allPlayers[p->id].y = p->y;
                        // allPlayers[p->id].id = p->id;
                        // }
                        // }
                    }
                }

                enet_packet_destroy(event.packet);
            }

            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "[Client] disconnected\n";
            ConnexionState = ClientState::DISCONNECTED;
            peer = nullptr;
            break;

        default:
            break;
        }
    }
}

void Client::sendMsg(ClientMsg msg)
{
    if (!peer)
        return;

    PacketHeader p;
    p.type = static_cast<uint8_t>(PacketType::CLIENT_MSG);
    p.code = static_cast<uint8_t>(msg);

    ENetPacket *packet = enet_packet_create(
        &p,
        sizeof(PacketHeader),
        ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(peer, 0, packet);
}
