#include "Client.hpp"
#include "../../World/Entities.hpp"

Client::Client()
    : clientHost(nullptr),
      peer(nullptr),
      ConnexionState(ClientState::DISCONNECTED),
      terrain() {}

Client::~Client() { stop(); }

bool Client::init()
{
    if (clientHost)
        return true;
    clientHost = enet_host_create(nullptr, 1, 2, 0, 0);
    allBullets.clear();
    allBonuses.clear();
    allEnemies.clear();
    allEnemiesTmp.clear();
    return clientHost != nullptr;
}

bool Client::start(const char *host, uint16_t port)
{
    if (!init())
        return false;

    ENetAddress address;
    enet_address_set_host(&address, host);
    address.port = port;

    peer = enet_host_connect(clientHost, &address, 2, 0);
    if (!peer)
        return false;

    ConnexionState = ClientState::CONNECTING;
    std::cout << "[Client] connecting to " << host << ":" << port << "\n";
    ClientTileRegistry::loadFromFile("assets/tiles/Tiles.yaml");

    for (auto &[id, arche] : EnemyArchetypeRegistry::archetypes)
    {
        ClientEnemyVisual v;
        if (!v.texture.loadFromFile(arche.texture))
        {
            std::cerr << "Failed to load texture: " << arche.texture << std::endl;
        }
        visuals[id] = std::move(v);
    }

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
    handleEnetService();
    {
        std::lock_guard<std::mutex> lock(mtx);
        }
    packetSendPosition();
}
