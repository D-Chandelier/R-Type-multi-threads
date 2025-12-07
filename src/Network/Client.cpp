#include "Client.hpp"

Client::Client() : clientHost(nullptr), peer(nullptr)
{
    enet_initialize();
    clientHost = enet_host_create(nullptr, 1, 2, 0, 0);
}

Client::~Client()
{
    if (clientHost)
        enet_host_destroy(clientHost);
    enet_deinitialize();
}

bool Client::connectTo(const char *host, uint16_t port)
{
    ENetAddress address;
    enet_address_set_host(&address, host);
    address.port = port;

    peer = enet_host_connect(clientHost, &address, 2, 0);
    if (!peer)
    {
        std::cerr << "[Client] no available peers for connection\n";
        return false;
    }

    ConnexionState = ClientState::CONNECTING;
    std::cout << "[Client] connecting to " << host << ":" << port << "\n";
    return true;
}

void Client::update(float dt)
{
    ENetEvent event;
    while (enet_host_service(clientHost, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            std::cout << "[Client] connected\n";
            ConnexionState = ClientState::CONNECTED;
            break;
        case ENET_EVENT_TYPE_RECEIVE:
        {
            const char *msg = reinterpret_cast<const char *>(event.packet->data);
            std::cout << "[Client] received: " << msg << "\n";
            enet_packet_destroy(event.packet);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "[Client] disconnected\n";
            ConnexionState = ClientState::DISCONNECTED;
            break;
        default:
            break;
        }
    }
}
