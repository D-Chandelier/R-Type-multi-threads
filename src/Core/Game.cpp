#include "Game.hpp"
// #include "../Network/Server.hpp"
// #include "../Network/Client.hpp"
// #include "../Core/MenuMain.hpp"
// #include "../Core/MenuServersList.hpp"
// #include "../Network/NetworkDiscovery.hpp"

// #include <SFML/Graphics.hpp>
// #include <thread>
// #include <chrono>
// #include <atomic>
// #include <iostream>

std::atomic<bool> running{true};

// Threads réseau
void runServer(Server *server)
{
    float dt = 0.016f;
    while (running)
    {
        server->update(dt);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void runClient(Client *client)
{
    float dt = 0.016f;
    while (running)
    {
        client->update(dt);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

Game::Game()
{
    state = GameState::MENU_MAIN;
}

void Game::run()
{
    sf::RenderWindow window(sf::VideoMode({Config::Get().windowSize}), Config::Get().title);
    MenuMain menuMain;
    MenuServersList MenuServersList;

    Server server;
    Client client;

    std::thread serverThread;
    std::thread clientThread;

    // --- BOUCLE PRINCIPALE ---
    while (window.isOpen())
    {
        switch (state)
        {
        case GameState::MENU_MAIN:
        {
            MenuResult result = menuMain.show(window);

            if (result == MenuResult::QUIT)
            {
                window.close();
            }
            else if (result == MenuResult::PLAY)
            {
                std::cout << "[LOCAL] Jeu en local\n";
                // DÉMARRER LE SERVEUR ICI
                server.start(1234);
                serverThread = std::thread(runServer, &server);

                // CONNECTER LE CLIENT LOCAL
                client.connectTo("127.0.0.1", Config::Get().serverPort);
                clientThread = std::thread(runClient, &client);

                state = GameState::GAME_LOCAL;
            }
            else if (result == MenuResult::JOIN)
            {
                std::cout << "[JOIN] Scanne le réseau...\n";

                if (MenuServersList.showServerList(window, client, Config::Get().discoveryPort) == MenuResult::NONE)
                {
                    state = GameState::MENU_MAIN;
                    continue;
                }
                state = GameState::GAME_CLIENT;
            }
            break;
        }

        case GameState::GAME_CLIENT:
        {
            // Jeu client simple (pas encore le niveau R-Type)
            while (auto eventOpt = window.pollEvent())
            {
                auto &event = *eventOpt;

                // Fermeture de la fenêtre
                if (auto *e = event.getIf<sf::Event::Closed>())
                {
                    running = false;
                    window.close();
                }
            }

            window.clear(Config::Get().backgroundColor);
            // ici tu dessineras ton R-Type plus tard
            window.display();
            break;
        }

        case GameState::GAME_LOCAL:
        {
            // Idem, futur mode solo
            while (auto eventOpt = window.pollEvent())
            {
                auto &event = *eventOpt;

                // Fermeture de la fenêtre
                if (auto *e = event.getIf<sf::Event::Closed>())
                {
                    running = false;
                    window.close();
                }
            }

            window.clear(Config::Get().backgroundColor);
            window.display();
            break;
        }

        default:
            break;
        }
    }

    // Quitter proprement
    running = false;
    if (serverThread.joinable())
        serverThread.join();
    if (clientThread.joinable())
        clientThread.join();
}
