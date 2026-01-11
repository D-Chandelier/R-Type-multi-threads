#include "Server.hpp"

Server::Server() : host(nullptr), positionAccumulator(0.0), lookahead(), cleanupMargin()
{
    lookahead = 3.f * Config::Get().windowSize.x;
    cleanupMargin = 2.f * Config::Get().windowSize.x;
}

Server::~Server() { stop(); }

bool Server::start(uint16_t port)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (host)
        stop();

    allPlayers.clear();
    allBullets.clear();
    nextBulletId = 0;

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    host = enet_host_create(&address, 32, 2, 0, 0);
    if (!host)
        return false;

    gameStartTime = Utils::localTimeNow();
    std::cout << "[Server] started on port " << port << " at " << gameStartTime << "\n";
    serverReady = true;

    levelSeed = std::random_device{}();
    levelTick = 0;
    worldX = 0.f;

    ServerTileRegistry::loadFromFile("assets/tiles/Tiles.yaml");
    EnemyArchetypeRegistry::loadFromFile("assets/entities/Entities.yaml");
    LevelRegistry::loadFromFile("assets/levels/Stage.yaml");
    LevelRegistry::setCurrent("level_01");

    const auto *levelDesc = LevelRegistry::current();
    if (!levelDesc)
    {
        std::cerr << "No current level set!\n";
        return false;
    }

    runtimeEnemies.clear();

    for (const auto &e : levelDesc->enemies)
    {
        runtimeEnemies.push_back(EnemyRuntime{e, false});
    }

    runtimeLevel.desc = LevelRegistry::current();
    runtimeLevel.nextSegmentIndex = 0;
    runtimeLevel.nextEnemyIndex = 0;

    terrain.rng.seed(LevelRegistry::current()->seed);

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
    allPlayers.clear();
    allBullets.clear();
    runtimeEnemies.clear();

    nextBulletId = 0;
    nextEnemyId = 0;

    serverReady = false;
}
