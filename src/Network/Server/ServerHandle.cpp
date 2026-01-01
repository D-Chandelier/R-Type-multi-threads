#include "Server.hpp"

void Server::handleEnetService(float dt)
{
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            handleTypeConnect(event);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            handleTypeReceive(event, dt);
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            handleTypeDisconnect(event);
            break;
        default:
            break;
        }
    }
}

void Server::handleTypeConnect(ENetEvent event)
{
    packetSendNewId(event);
}

void Server::handleTypeReceive(ENetEvent event, float dt)
{
    if (!event.packet)
        return;

    PacketHeader *h = (PacketHeader *)event.packet->data;
    if (h->type != static_cast<uint8_t>(PacketType::CLIENT_MSG))
        return;
    switch (h->code)
    {
    case static_cast<uint8_t>(ClientMsg::PLAYER_POSITION):
        packetReceivedPlayerPosition(event, dt);
        break;
    case static_cast<uint8_t>(ClientMsg::BULLET_SHOOT):
        packetReceivedBulletShoot(event);
        break;
    case static_cast<uint8_t>(ClientMsg::REJOIN):
        packetReceivedRejoin(event);
        break;
    default:
        return;
    }
}

void Server::handleTypeDisconnect(ENetEvent event)
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
}
