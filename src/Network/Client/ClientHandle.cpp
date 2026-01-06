#include "Client.hpp"

void Client::handleEnetService()
{
    ENetEvent event;
    while (enet_host_service(clientHost, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            handleTypeConnect(event);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            handleTypeReceive(event);
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

void Client::handleTypeReceive(ENetEvent &event)
{
    if (event.packet)
    {
        PacketHeader *h = (PacketHeader *)event.packet->data;

        if (h->type == static_cast<uint8_t>(PacketType::SERVER_MSG))
        {
            switch (h->code)
            {
            case static_cast<uint8_t>(ServerMsg::ASSIGN_ID):
                packetReceivedId(event);
                break;
            case static_cast<uint8_t>(ServerMsg::PLAYER_POSITION):
                packetReceivedPlayersPositions(event);
                break;
            case static_cast<uint8_t>(ServerMsg::BULLET_SHOOT):
                packetReceivedBullets(event);
                break;
            case static_cast<uint8_t>(ServerMsg::ROCKET_STATE):
                packetReceivedRocketState(event);
                break;
            case static_cast<uint8_t>(ServerMsg::INIT_LEVEL):
                packetReceivedInitLevel(event);
                break;
            case static_cast<uint8_t>(ServerMsg::WORLD_X_UPDATE):
                packetReceivedWorldX(event);
                break;
            case static_cast<uint8_t>(ServerMsg::NEW_SEGMENT):
                packetReceivedSegment(event);
                break;
            case static_cast<uint8_t>(ServerMsg::ALL_SEGMENTS):
                packetReceivedAllSegments(event);
                break;
            case static_cast<uint8_t>(ServerMsg::ENEMY_DESTROYED):
                packetReceivedEnemyDestroyed(event);
                break;
            case static_cast<uint8_t>(ServerMsg::BULLET_DESTROYED):
                packetReceivedBulletDestroyed(event);
                break;
            case static_cast<uint8_t>(ServerMsg::ENEMIES):
                packetReceivedEnemies(event);
                break;
            case static_cast<uint8_t>(ServerMsg::BONUS_SPAWN):
                packetReceivedBonusSpawn(event);
                break;
            case static_cast<uint8_t>(ServerMsg::BONUS_DESTROYED):
                packetReceivedBonusDestroy(event);
                break;

            default:
                return;
            }
        }
    }
}

void Client::handleTypeConnect(ENetEvent &event)
{
    std::cout << "[Client] connected\n";
    peer = event.peer;
    ConnexionState = ClientState::CONNECTING;

    PacketHeader p;
    p.type = static_cast<uint8_t>(PacketType::CLIENT_MSG);

    if (Config::Get().isServer)
        p.code = static_cast<uint8_t>(ClientMsg::REQUEST_NEW_GAME);
    else
        p.code = static_cast<uint8_t>(ClientMsg::REQUEST_JOIN_GAME);

    ENetPacket *packet = enet_packet_create(
        &p,
        sizeof(PacketHeader),
        ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(peer, 0, packet);
}

void Client::handleTypeDisconnect(ENetEvent &event)
{
    std::cout << "[Client] disconnected\n";
    ConnexionState = ClientState::DISCONNECTED;
    peer = nullptr;
}