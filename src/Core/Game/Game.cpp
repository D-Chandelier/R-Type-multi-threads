#include "Game.hpp"

Game::Game()
    : window(sf::VideoMode(Config::Get().windowSize),
             Config::Get().title),
      menuMain(),
      menuServers(client, discoveryClient),
      menuOption(),
      menuInGame(),
      menuBackground(Config::Get().font)
{
    window.setFramerateLimit(Config::Get().frameRate);
    window.setVerticalSyncEnabled(true);

    currentMenu = &menuMain;

    playersVA.setPrimitiveType(sf::PrimitiveType::Triangles);
    playersVA.resize(6 * MAX_PLAYER);

    backgroundVA_1.setPrimitiveType(sf::PrimitiveType::Triangles);
    backgroundVA_1.resize(6);
    backgroundVA_2.setPrimitiveType(sf::PrimitiveType::Triangles);
    backgroundVA_2.resize(6);

    Bullet::Get().bulletsVA.setPrimitiveType(sf::PrimitiveType::Triangles);
    Bullet::Get().rocketsVA.setPrimitiveType(sf::PrimitiveType::Triangles);

    float w = static_cast<float>(Config::Get().windowSize.x);
    float h = static_cast<float>(Config::Get().windowSize.y);

    backgroundVA_1[0].position = {0.f, 0.f};
    backgroundVA_1[1].position = {w, 0.f};
    backgroundVA_1[2].position = {w, h};
    backgroundVA_2[0].position = {0.f, 0.f};
    backgroundVA_2[1].position = {w, 0.f};
    backgroundVA_2[2].position = {w, h};

    backgroundVA_1[3].position = {0.f, 0.f};
    backgroundVA_1[4].position = {w, h};
    backgroundVA_1[5].position = {0.f, h};
    backgroundVA_2[3].position = {0.f, 0.f};
    backgroundVA_2[4].position = {w, h};
    backgroundVA_2[5].position = {0.f, h};
}

void Game::runServer()
{
    using clock = std::chrono::steady_clock;
    auto last = clock::now();

    while (running.load())
    {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        dt = std::min(dt, 0.1f);
        last = now;

        server.update(dt);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Game::runClient()
{
    using clock = std::chrono::steady_clock;
    auto last = clock::now();

    const float targetFrameTime = 1.f / Config::Get().frameRate;

    while (running)
    {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        dt = std::min(dt, 0.1f);

        client.update(dt);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Game::stopThreads()
{
    running.store(false);

    if (serverThread.joinable())
        serverThread.join();
    if (clientThread.joinable())
        clientThread.join();

    server.stop();
    client.stop();

    discoveryClient.stopBroadcast();
    discoveryClient.stopListening();
}

void Game::run()
{
    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        handleEvent();
        handleEventPlayerMove(dt);

        update(dt);
        draw(dt);
    }
}
