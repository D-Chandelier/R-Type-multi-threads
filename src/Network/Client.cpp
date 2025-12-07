#include "Client.hpp"
#include <iostream>
#include <cstring>
#include "NetworkHelpers.hpp"

Client::Client() {}
Client::~Client() { stop(); }

bool Client::start()
{
    if (enet_initialize() != 0)
    {
        std::cerr << "[Client] enet_initialize failed\n";
        return false;
    }
    clientHost = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!clientHost)
    {
        std::cerr << "[Client] enet_host_create failed\n";
        enet_deinitialize();
        return false;
    }
    running = true;
    netThread = std::thread(&Client::networkThreadFunc, this);
    return true;
}

void Client::stop()
{
    bool was = running.exchange(false);
    if (!was)
        return;
    if (netThread.joinable())
        netThread.join();
    if (clientHost)
    {
        enet_host_destroy(clientHost);
        clientHost = nullptr;
    }
    enet_deinitialize();
}

bool Client::connectTo(const char *hostaddr, uint16_t port)
{
    if (!clientHost)
        start();

    ENetAddress addr;
    enet_address_set_host(&addr, hostaddr);
    addr.port = port;

    serverPeer = enet_host_connect(clientHost, &addr, 2, 0);
    if (!serverPeer)
    {
        std::cerr << "[Client] no available peers for connection\n";
        return false;
    }

    // Optionnel: attendre confirmation de connect
    ENetEvent event;
    if (enet_host_service(clientHost, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
    {
        std::cout << "[Client] connected\n";
        return true;
    }
    else
    {
        std::cerr << "[Client] connection timeout\n";
        return false;
    }
}

void Client::disconnect()
{
    if (serverPeer)
    {
        enet_peer_disconnect(serverPeer, 0);
        serverPeer = nullptr;
    }
}

void Client::sendInput(const ClientInput &in)
{
    outInputs.push(in);
}

void Client::setSnapshotCallback(SnapshotCallback cb)
{
    snapshotCb = cb;
}

void Client::networkThreadFunc()
{
    ENetEvent event;
    while (running)
    {
        // 1) envoyer inputs en attente
        ClientInput out;
        while (outInputs.try_pop(out))
        {
            ENetPacket *packet = enet_packet_create(&out, sizeof(ClientInput), ENET_PACKET_FLAG_UNSEQUENCED);
            if (serverPeer)
                enet_peer_send(serverPeer, 0, packet);
            enet_host_flush(clientHost);
        }

        // 2) lire événements serveur
        while (enet_host_service(clientHost, &event, 5) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
            {
                // parse snapshot
                SnapshotHeader header;
                std::vector<PlayerState> players;
                if (deserializeSnapshot(event.packet->data, event.packet->dataLength, header, players))
                {
                    if (snapshotCb)
                        snapshotCb(header.tick, players);
                }
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "[Client] disconnected by server\n";
                serverPeer = nullptr;
                break;
            default:
                break;
            }
        }
    }
}
