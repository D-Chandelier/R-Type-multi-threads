#include "Client.hpp"

Client::Client()
    : clientHost(nullptr),
      peer(nullptr),
      ConnexionState(ClientState::DISCONNECTED)
{
    if (!localPlayer.texture.loadFromFile("assets/r-typesheet42.gif"))
        std::cout << "Erreur loading r-typesheet42.gif" << "\n";

    localPlayer.sprite.setTexture(localPlayer.texture);

    // Taille d'une cellule
    int cellWidth = localPlayer.sprite.getTexture().getSize().x / 5;
    int cellHeight = localPlayer.sprite.getTexture().getSize().y / 5;
    localPlayer.sprite.setTextureRect(sf::IntRect(
        {2 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}));

    if (!backgroundTexture1.loadFromFile("assets/Starfield_07-1024x1024.png"))
    {
        std::cerr << "Erreur chargement texture background 1\n";
    }
    backgroundTexture1.setRepeated(true); // optionnel mais propre
    if (!backgroundTexture2.loadFromFile("assets/Blue_Nebula_08-1024x1024.png"))
    {
        std::cerr << "Erreur chargement texture background 2\n";
    }
    backgroundTexture2.setRepeated(true);

    playersVA.setPrimitiveType(sf::PrimitiveType::Triangles);
    playersVA.resize(6 * Config::Get().maxPlayers);

    backgroundVA_1.setPrimitiveType(sf::PrimitiveType::Triangles);
    backgroundVA_1.resize(6); // 2 triangles
    backgroundVA_2.setPrimitiveType(sf::PrimitiveType::Triangles);
    backgroundVA_2.resize(6); // 2 triangles

    // Définir positions fixes de l'écran
    float w = static_cast<float>(Config::Get().windowSize.x);
    float h = static_cast<float>(Config::Get().windowSize.y);

    // 1er triangle
    backgroundVA_1[0].position = {0.f, 0.f};
    backgroundVA_1[1].position = {w, 0.f};
    backgroundVA_1[2].position = {w, h};
    backgroundVA_2[0].position = {0.f, 0.f};
    backgroundVA_2[1].position = {w, 0.f};
    backgroundVA_2[2].position = {w, h};

    // 2e triangle
    backgroundVA_1[3].position = {0.f, 0.f};
    backgroundVA_1[4].position = {w, h};
    backgroundVA_1[5].position = {0.f, h};
    backgroundVA_2[3].position = {0.f, 0.f};
    backgroundVA_2[4].position = {w, h};
    backgroundVA_2[5].position = {0.f, h};
}

Client::~Client() { stop(); }

bool Client::init()
{
    if (clientHost)
        return true; // déjà créé
    clientHost = enet_host_create(nullptr, 1, 2, 0, 0);
    return clientHost != nullptr;
}

double Client::localTimeNow() const
{
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
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

void Client::eventReceivePlayersPositions(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ServerPositionPacket))
        return;

    auto *p = reinterpret_cast<ServerPositionPacket *>(event.packet->data);

    std::unordered_set<int> seenIds;

    for (int i = 0; i < p->playerCount; i++)
    {
        int pid = p->players[i].id;

        // if (pid == localPlayer.id)
        // {
        //     // Mettre à jour seulement la position locale si nécessaire
        //     localPlayer.position.x = p->players[i].x;
        //     localPlayer.position.y = p->players[i].y;
        //     continue;
        // }

        seenIds.insert(pid);

        RemotePlayer &rp = allPlayers[pid];
        rp.id = pid;
        rp.lastUpdateTime = p->serverGameTime;
        rp.serverX = p->players[i].x;
        rp.serverY = p->players[i].y;
    }

    // Supprimer uniquement les joueurs distants qui ne sont plus dans le snapshot
    for (auto it = allPlayers.begin(); it != allPlayers.end();)
    {
        if (it->first != localPlayer.id && seenIds.find(it->first) == seenIds.end())
            it = allPlayers.erase(it);
        else
            ++it;
    }

    serverTimeOffset = p->serverGameTime - localTimeNow();
    backgroundScrollSpeed = p->scrollSpeed;
}

void Client::eventReceiveId(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ServerAssignIdPacket))
        return;

    auto *p = reinterpret_cast<ServerAssignIdPacket *>(event.packet->data);

    allPlayers[p->id].id = p->id;
    localPlayer.id = p->id;
    localPlayer.position = {Config::Get().windowSize.x / 20.f, Config::Get().windowSize.y / (static_cast<float>(Config::Get().maxPlayers) + 1.f) * (static_cast<float>(p->id) + 1.f)};
    sendPosition();
    ConnexionState = ClientState::CONNECTED;

    std::cout << "[Client] received(ASSIGN_ID): [" << localPlayer.id << "] \n";
}

void Client::handleTypeConnect(ENetEvent event)
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

void Client::handleTypeReceive(ENetEvent event)
{
    if (event.packet)
    {
        PacketHeader *h = (PacketHeader *)event.packet->data;

        if (h->type == static_cast<uint8_t>(PacketType::SERVER_MSG))
        {
            switch (h->code)
            {
            case static_cast<uint8_t>(ServerMsg::ASSIGN_ID):
                eventReceiveId(event);

            case static_cast<uint8_t>(ServerMsg::PLAYER_POSITION):
                eventReceivePlayersPositions(event);

            default:
                return;
            }
        }
    }
}

void Client::handleTypeDisconnect(ENetEvent event)
{
    std::cout << "[Client] disconnected\n";
    ConnexionState = ClientState::DISCONNECTED;
    peer = nullptr;
}

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

void Client::update(float dt)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (!clientHost)
        return;

    sendPosition();
    updateBackgrounds();
    updatePlayers();
    handleEnetService();
}

void Client::sendPosition()
{
    // envoyer la position au serveur
    if (localPlayer.id < 0 || localPlayer.id > Config::Get().maxPlayers)
        return;

    if (peer)
    {

        // Clamp pour ne pas sortir de l'écran
        float halfW = localPlayer.sprite.getLocalBounds().getCenter().x * localPlayer.sprite.getScale().x;
        float halfH = localPlayer.sprite.getLocalBounds().getCenter().y * localPlayer.sprite.getScale().y;

        localPlayer.position.x = std::clamp(
            localPlayer.position.x,
            halfW,
            Config::Get().windowSize.x - halfW);
        localPlayer.position.y = std::clamp(
            localPlayer.position.y,
            halfH,
            Config::Get().windowSize.y - halfH);
        ClientPositionPacket p;
        p.header.type = static_cast<uint8_t>(PacketType::CLIENT_MSG);
        p.header.code = static_cast<uint8_t>(ClientMsg::PLAYER_POSITION);
        p.id = localPlayer.id;
        p.x = localPlayer.position.x;
        p.y = localPlayer.position.y;
        // std::cout
        //     << "[PLAYER] sendPosition(PLAYER_POSITION):\n"
        //     << "Id: " << p.id << "\n"
        //     << "Poition > x: " << p.x << ", y: " << p.y << "\n";

        ENetPacket *packet = enet_packet_create(&p, sizeof(p), 0);
        enet_peer_send(peer, 0, packet);
        // enet_host_flush(client.clientHost);
    }
}

void Client::updateBackgrounds()
{

    float texW = static_cast<float>(backgroundTexture1.getSize().x);
    float texH = static_cast<float>(backgroundTexture1.getSize().y);
    sf::Vector2f winSize = {static_cast<float>(Config::Get().windowSize.x), static_cast<float>(Config::Get().windowSize.y)};

    double gameTime = localTimeNow() + serverTimeOffset;

    background_1_OffsetX = std::fmod(
        static_cast<float>(gameTime) * backgroundScrollSpeed,
        texH);
    background_2_OffsetX = std::fmod(
        static_cast<float>(gameTime) * backgroundScrollSpeed * 2.f,
        texH);

    // offset horizontal ou vertical
    float offsetX = background_1_OffsetX; // déjà calculé
    float offsetY = background_1_OffsetY; // pour vertical

    // Scroll horizontal + vertical, en boucle grâce à setRepeated(true)
    backgroundVA_1[0].texCoords = {offsetX, offsetY};
    backgroundVA_1[1].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY};
    backgroundVA_1[2].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_1[3].texCoords = {offsetX, offsetY};
    backgroundVA_1[4].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_1[5].texCoords = {offsetX, offsetY + static_cast<float>(winSize.y)};
    offsetX = background_2_OffsetX; // déjà calculé
    offsetY = background_2_OffsetY; // pour vertical
    backgroundVA_2[0].texCoords = {offsetX, offsetY};
    backgroundVA_2[1].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY};
    backgroundVA_2[2].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_2[3].texCoords = {offsetX, offsetY};
    backgroundVA_2[4].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_2[5].texCoords = {offsetX, offsetY + static_cast<float>(winSize.y)};

    sf::Color alphaColor(255, 255, 255, 128); // 50% transparent

    for (size_t i = 0; i < backgroundVA_2.getVertexCount(); ++i)
        backgroundVA_2[i].color = alphaColor; // 50% transparent
}
void Client::updatePlayers()
{
    for (auto &[id, p] : allPlayers)
    {
        // if (id != localPlayer.id)
        // {
        // Temps écoulé depuis la dernière position serveur
        double delta = localTimeNow() - p.lastUpdateTime;
        // Interpolation simple
        float alpha = static_cast<float>(delta / (1.0f / Config::Get().frameRate)); // delta / tick serveur (xx ms)
        alpha = std::clamp(alpha, 0.f, 1.f);

        p.x += (p.serverX - p.x) * alpha;
        p.y += (p.serverY - p.y) * alpha;
        // }
        // else
        // {
        //     p.x = localPlayer.position.x;
        //     p.y = localPlayer.position.y;
        // }
    }

    int i = 0;
    int cellWidth = localPlayer.texture.getSize().x / 5;
    int cellHeight = localPlayer.texture.getSize().y / 5;

    playersVA.resize(allPlayers.size() * 6);

    for (int i = 0; i < allPlayers.size(); ++i)
    {
        const auto &[id, p] = *std::next(allPlayers.begin(), i);

        sf::Vector2f origin = localPlayer.sprite.getOrigin();
        sf::Vector2f scale = localPlayer.sprite.getScale();

        float w = static_cast<float>(cellWidth) * scale.x;
        float h = static_cast<float>(cellHeight) * scale.y;

        float x = (p.x - origin.x * scale.x);
        float y = (p.y - origin.y * scale.y);

        // positions du quad (2 triangles)
        playersVA[i * 6 + 0].position = {x, y};
        playersVA[i * 6 + 1].position = {x + w, y};
        playersVA[i * 6 + 2].position = {x + w, y + h};

        playersVA[i * 6 + 3].position = {x, y};
        playersVA[i * 6 + 4].position = {x + w, y + h};
        playersVA[i * 6 + 5].position = {x, y + h};

        // texCoords selon sprite dans spritesheet
        float tx = 2 * cellWidth;
        float ty = id * cellHeight;
        playersVA[i * 6 + 0].texCoords = {tx, ty};
        playersVA[i * 6 + 1].texCoords = {tx + cellWidth, ty};
        playersVA[i * 6 + 2].texCoords = {tx + cellWidth, ty + cellHeight};
        playersVA[i * 6 + 3].texCoords = {tx, ty};
        playersVA[i * 6 + 4].texCoords = {tx + cellWidth, ty + cellHeight};
        playersVA[i * 6 + 5].texCoords = {tx, ty + cellHeight};
    }
}

void Client::drawPlayers(sf::RenderWindow &w)
{
    sf::RenderStates states;
    states.texture = &Config::Get().texture;
    w.draw(playersVA, states);
}

void Client::drawBackground(sf::RenderWindow &w)
{
    sf::RenderStates states_1, states_2;
    states_1.texture = &backgroundTexture1;
    states_2.texture = &backgroundTexture2;

    w.draw(backgroundVA_1, states_1);
    w.draw(backgroundVA_2, states_2);
}
