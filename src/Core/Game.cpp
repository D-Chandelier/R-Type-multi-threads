#include "Game.hpp"

std::atomic<bool> running{true};

// Threads réseau
void runServer(Server *server)
{
    using namespace std::chrono_literals;

    while (running)
    {
        server->update(Config::Get().frameRate);
        std::this_thread::sleep_for(16ms);
    }
}

void runClient(Client *client)
{
    using namespace std::chrono_literals;

    while (running)
    {
        client->update(Config::Get().frameRate);
        std::this_thread::sleep_for(16ms);
    }
}

Game::Game()
    : window(sf::VideoMode(Config::Get().windowSize),
             Config::Get().title),
      menuMain(),
      menuServers(client, discoveryClient),
      menuOption(),
      menuInGame()
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
                    if (state == GameState::IN_GAME)
                    {
                        state = GameState::MENU_IN_GAME;
                        currentMenu = &menuInGame;
                        currentMenu->reset();
                    }
                    else if (state == GameState::MENU_IN_GAME)
                    {
                        state = GameState::IN_GAME;
                        currentMenu = nullptr;
                    }
                    else if (state == GameState::MENU_JOIN || state == GameState::MENU_OPTION)
                    {
                        stopThreads();
                        state = GameState::MENU_MAIN;
                        currentMenu = &menuMain;
                        currentMenu->reset();
                    }
                }
            }

            if (currentMenu && state != GameState::IN_GAME)
                currentMenu->handleEvent(*event, window);
        }

        // -------------------------------------
        // 2. UPDATE
        // -------------------------------------
        if (currentMenu && state != GameState::IN_GAME)
        {
            currentMenu->update(dt, window);
            handleMenuAction(); // <-- important
        }
        else if (state == GameState::IN_GAME)
        {
            updateGameplay(dt); // <-- ta future logique in-game
        }
        // else if (state == GameState::MENU_IN_GAME)
        // {
        // }

        bg.update(dt);

        // -------------------------------------
        // 3. DRAW
        // -------------------------------------
        window.clear(Config::Get().backgroundColor);

        if (state != GameState::IN_GAME)
        {
            bg.draw(window);
            currentMenu->draw(window);
        }
        else
            drawGameplay(window);

        window.display();
    }
}

void Game::stopThreads()
{
    // Demande l'arrêt aux threads
    running = false;

    // Attend la fin des threads réseau AVANT de détruire quoi que ce soit
    if (serverThread.joinable())
        serverThread.join();
    if (clientThread.joinable())
        clientThread.join();

    // Maintenant, les threads sont terminés — on peut détruire les hosts en toute sécurité
    server.stop();
    client.stop();

    // Réinitialiser discovery (UDP)
    discoveryClient.stopBroadcast();
    discoveryClient.stopListening();
}

void Game::handleMenuAction()
{
    if (!currentMenu)
        return;
    // Vérifie si le menu veut changer l’écran
    MenuAction a = currentMenu->getAction();
    if (a == MenuAction::NONE)
        return;

    switch (a)
    {
    case MenuAction::GO_TO_MAIN_MENU:
        stopThreads();
        state = GameState::MENU_MAIN;
        menuMain.reset();
        currentMenu = &menuMain;
        break;

    case MenuAction::GO_TO_SERVER_LIST:
        state = GameState::MENU_JOIN;
        menuServers.reset();
        currentMenu = &menuServers;
        discoveryClient.startListening(Config::Get().discoveryPort);
        break;

    case MenuAction::GO_TO_OPTION_MENU:
        state = GameState::MENU_OPTION;
        menuOption.reset();
        currentMenu = &menuOption;
        break;

    case MenuAction::START_GAME:
        stopThreads();

        Config::Get().isServer = true;
        running = true;

        // Démarrage serveur
        server.start(Config::Get().serverPort);
        serverThread = std::thread(runServer, &server);
        discoveryClient.startBroadcast(Config::Get().serverPort, Config::Get().discoveryPort);

        // Connexion client local
        client.connectTo("127.0.0.1", Config::Get().serverPort);
        clientThread = std::thread(runClient, &client);

        // IMPORTANT : on ne passe PAS encore en gameplay logique
        state = GameState::IN_GAME;

        // Dès que la connexion est établie, le client enverra REQUEST_NEW_GAME
        break;

    case MenuAction::JOIN_SERVER:
        stopThreads();

        Config::Get().isServer = false;
        running = true;

        client.connectTo(Config::Get().serverIp.c_str(), Config::Get().serverPort);
        clientThread = std::thread(runClient, &client);

        state = GameState::IN_GAME;
        break;

    case MenuAction::RETURN_IN_GAME:
        state = GameState::IN_GAME;
        menuInGame.reset();
        currentMenu = nullptr;
        break;

    case MenuAction::QUIT_APP:
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
