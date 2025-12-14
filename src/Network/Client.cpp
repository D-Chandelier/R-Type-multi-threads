#include "Client.hpp"

Client::Client()
    : clientHost(nullptr),
      peer(nullptr),
      ConnexionState(ClientState::DISCONNECTED)
{
    std::lock_guard<std::mutex> lock(mtx);
    clientHost = enet_host_create(nullptr, 1, 2, 0, 0);
}

Client::~Client()
{
    stop();
}

bool Client::init()
{
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
        enet_peer_disconnect(peer, 0);
        peer = nullptr;
    }

    if (clientHost)
    {
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
            std::cout << "[Client] connected\n";
            peer = event.peer;
            ConnexionState = ClientState::CONNECTED;

            if (Config::Get().isServer)
                sendMsg(ClientMsg::REQUEST_NEW_GAME);
            else
                sendMsg(ClientMsg::REQUEST_JOIN_GAME);
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            if (event.packet)
            {
                const char *msg = reinterpret_cast<const char *>(event.packet->data);
                std::cout << "[Client] received: " << msg << "\n";
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
    p.type = PacketType::CLIENT_MSG;
    p.code = static_cast<uint8_t>(msg);

    ENetPacket *packet = enet_packet_create(
        &p,
        sizeof(PacketHeader),
        ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(peer, 0, packet);
}
