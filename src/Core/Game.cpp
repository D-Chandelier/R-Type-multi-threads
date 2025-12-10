#include "Game.hpp"

std::atomic<bool> running{true};

// Threads réseau
void runServer(Server *server)
{
    float dt = Config::Get().frameRate;
    while (running)
    {
        server->update(dt);
        std::this_thread::sleep_for(std::chrono::seconds(static_cast<long>(dt)));
    }
}

void runClient(Client *client)
{
    float dt = Config::Get().frameRate;
    while (running)
    {
        client->update(dt);
        std::this_thread::sleep_for(std::chrono::seconds(static_cast<long>(dt)));
    }
}

Game::Game()
    : window(sf::VideoMode(Config::Get().windowSize),
             Config::Get().title),
      menuMain(),
      menuServers(client)
{
    currentMenu = &menuMain;
}

void Game::run()
{
    sf::Clock clock;
    Background bg(Config::Get().font);

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        // -------------------------------------
        // 1. EVENTS
        // -------------------------------------
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                stopThreads();
                window.close();
            }
            if (auto *e = event->getIf<sf::Event::KeyPressed>())
            {
                if (e->code == sf::Keyboard::Key::Escape)
                {
                    std::cout << "ESC pressed - state=" << (int)state << std::endl;
                    if (state == GameState::MENU_JOIN)
                    {
                        state = GameState::MENU_MAIN;
                        currentMenu = &menuMain;
                        currentMenu->reset();
                    }
                    else if (state == GameState::IN_GAME)
                    {                                 // state = GameState::MENU_IN_GAME;
                        state = GameState::MENU_MAIN; // pour test
                        currentMenu = &menuMain;
                        currentMenu->reset();
                    }
                }
            }

            if (currentMenu)
                currentMenu->handleEvent(*event, window);
        }

        // -------------------------------------
        // 2. UPDATE
        // -------------------------------------
        if (currentMenu)
        {
            currentMenu->update(dt, window);
            handleMenuAction(); // <-- important
        }
        else if (state == GameState::IN_GAME)
        {
            updateGameplay(dt); // <-- ta future logique in-game
        }
        else if (state == GameState::MENU_IN_GAME)
        {
        }

        bg.update(dt);

        // -------------------------------------
        // 3. DRAW
        // -------------------------------------
        window.clear(Config::Get().backgroundColor);
        bg.draw(window);

        if (state == GameState::MENU_MAIN || state == GameState::MENU_JOIN)
            currentMenu->draw(window);
        else if (state == GameState::IN_GAME)
            drawGameplay(window);

        window.display();
    }
}

void Game::stopThreads()
{
    running = false;
    if (serverThread.joinable())
        serverThread.join();
    if (clientThread.joinable())
        clientThread.join();
}

void Game::handleMenuAction()
{
    // Vérifie si le menu veut changer l’écran
    MenuAction a = currentMenu->getAction();

    switch (a)
    {
    case MenuAction::GO_TO_MAIN_MENU:
        state = GameState::MENU_MAIN;
        menuMain.reset();
        currentMenu = &menuMain;
        break;

    case MenuAction::GO_TO_SERVER_LIST:
        state = GameState::MENU_JOIN;
        menuServers.reset();
        currentMenu = &menuServers;
        break;

    case MenuAction::START_GAME:
        state = GameState::IN_GAME;
        // DÉMARRER LE SERVEUR
        server.start(Config::Get().serverPort);
        serverThread = std::thread(runServer, &server);

        // CONNECTER EN CLIENT LOCAL
        client.connectTo("127.0.0.1", Config::Get().serverPort);
        clientThread = std::thread(runClient, &client);

        currentMenu->reset();
        break;

    case MenuAction::JOIN_SERVER:
        state = GameState::IN_GAME;
        // CONNECTER EN CLIENT LOCAL
        client.connectTo(Config::Get().serverIp.c_str(), Config::Get().serverPort);
        clientThread = std::thread(runClient, &client);
        currentMenu->reset();
        break;

    case MenuAction::QUIT_APP:
        // Quitter
        stopThreads();
        window.close();
        break;

    default:
        break;
    }
}

void Game::updateGameplay(float dt)
{
    // Todo: Logique R-Type in Game
}

void Game::drawGameplay(sf::RenderWindow &w)
{
    // ToDo: Draw R-Type in game
}

// void Game::run()
// {
//     sf::Clock clock;

//     Background bg(Config::Get().font);

//     while (window.isOpen())
//     {
//         float dt = clock.restart().asSeconds();

//         while (auto e = window.pollEvent())
//         {
//             if (auto *c = e->getIf<sf::Event::Closed>())
//             {
//                 running = false;
//                 if (serverThread.joinable())
//                     serverThread.join();
//                 if (clientThread.joinable())
//                     clientThread.join();
//                 window.close();
//             }
//             currentMenu->handleEvent(*e, window);
//         }

//         currentMenu->update(dt, window);
//         bg.update(dt);

//         // Vérifie si le menu veut changer l’écran
//         MenuAction a = currentMenu->getAction();

//         switch (a)
//         {
//         case MenuAction::GO_TO_MAIN_MENU:
//             menuMain.reset();
//             currentMenu = &menuMain;
//             break;

//         case MenuAction::GO_TO_SERVER_LIST:
//             menuServers.reset();
//             currentMenu = &menuServers;
//             break;

//         case MenuAction::START_GAME:
//             // DÉMARRER LE SERVEUR
//             server.start(Config::Get().serverPort);
//             serverThread = std::thread(runServer, &server);

//             // CONNECTER EN CLIENT LOCAL
//             client.connectTo("127.0.0.1", Config::Get().serverPort);
//             clientThread = std::thread(runClient, &client);

//             currentMenu->reset();
//             state = GameState::IN_GAME;
//             break;

//         case MenuAction::JOIN_SERVER:
//             // CONNECTER EN CLIENT LOCAL
//             client.connectTo(Config::Get().serverIp.c_str(), Config::Get().serverPort);
//             clientThread = std::thread(runClient, &client);
//             currentMenu->reset();
//             state = GameState::IN_GAME;
//             break;

//         case MenuAction::QUIT_APP:
//             // Quitter
//             running = false;
//             if (serverThread.joinable())
//                 serverThread.join();
//             if (clientThread.joinable())
//                 clientThread.join();
//             window.close();
//             break;

//         default:
//             break;
//         }

//         window.clear(Config::Get().backgroundColor);
//         bg.draw(window);
//         currentMenu->draw(window);
//         window.display();
//     }
// }

// #include "Game.hpp"
// // #include "../Network/Server.hpp"
// // #include "../Network/Client.hpp"
// // #include "../Core/MenuMain.hpp"
// // #include "../Core/MenuServersList.hpp"
// // #include "../Network/NetworkDiscovery.hpp"

// // #include <SFML/Graphics.hpp>
// // #include <thread>
// // #include <chrono>
// // #include <atomic>
// // #include <iostream>

// std::atomic<bool> running{true};

// // Threads réseau
// void runServer(Server *server)
// {
//     float dt = Config::Get().frameRate;
//     while (running)
//     {
//         server->update(dt);
//         std::this_thread::sleep_for(std::chrono::seconds(static_cast<long>(dt)));
//     }
// }

// void runClient(Client *client)
// {
//     float dt = Config::Get().frameRate;
//     while (running)
//     {
//         client->update(dt);
//         std::this_thread::sleep_for(std::chrono::seconds(static_cast<long>(dt)));
//     }
// }

// Game::Game()
// {
//     state = GameState::MENU_MAIN;
// }

// void Game::run()
// {
//     sf::RenderWindow window(sf::VideoMode({Config::Get().windowSize}), Config::Get().title);

//     // --- BOUCLE PRINCIPALE ---
//     while (window.isOpen())
//     {
//         switch (state)
//         {
//         case GameState::MENU_MAIN:
//         {
//             MenuResult result = menuMain.show(window);

//             if (result == MenuResult::QUIT)
//                 window.close();

//             if (result == MenuResult::PLAY)
//             {
//                 // DÉMARRER LE SERVEUR
//                 server.start(Config::Get().serverPort);
//                 serverThread = std::thread(runServer, &server);

//                 // CONNECTER EN CLIENT LOCAL
//                 client.connectTo("127.0.0.1", Config::Get().serverPort);
//                 clientThread = std::thread(runClient, &client);

//                 state = GameState::IN_GAME;
//             }

//             if (result == MenuResult::JOIN)
//             {
//                 if (menuServersList.showServerList(window, client, Config::Get().discoveryPort) == MenuResult::NONE)
//                 {
//                     state = GameState::MENU_MAIN;
//                     continue;
//                 }
//                 state = GameState::IN_GAME;
//             }
//             break;
//         }

//         case GameState::IN_GAME:
//         {
//             while (auto eventOpt = window.pollEvent())
//             {
//                 auto &event = *eventOpt;

//                 // Fermeture de la fenêtre
//                 if (auto *e = event.getIf<sf::Event::Closed>())
//                 {
//                     running = false;
//                     window.close();
//                 }
//             }

//             window.clear(Config::Get().backgroundColor);
//             // ToDo: dessiner R-Type
//             window.display();
//             break;
//         }

//         default:
//             break;
//         }
//     }

//     // Quitter
//     running = false;
//     if (serverThread.joinable())
//         serverThread.join();
//     if (clientThread.joinable())
//         clientThread.join();
// }
