#include "Game.hpp"

void Game::handleEvent()
{
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
        running.store(true); // = true;

        // --- Démarrage serveur ---
        if (!server.start(Config::Get().serverPort))
        {
            std::cerr << "[Game] Impossible de démarrer le serveur.\n";
            break;
        }
        serverThread = std::thread(&Game::runServer, this); //&server);

        while (!server.serverReady)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (!client.start("127.0.0.1", Config::Get().serverPort))
        {
            std::cerr << "[Game] Impossible de connecter le client.\n";
            break;
        }

        clientThread = std::thread(&Game::runClient, this); // &client);

        discoveryClient.startBroadcast(Config::Get().serverPort, Config::Get().discoveryPort);

        state = GameState::IN_GAME;
        break;

    case MenuAction::JOIN_SERVER:
        stopThreads();

        Config::Get().isServer = false;
        running.store(true); // = true;

        client.start(Config::Get().serverIp.c_str(), Config::Get().serverPort);
        clientThread = std::thread(&Game::runClient, this); // &client);

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

void Game::handleEventPlayerMove(float dt)
{
    if (state != GameState::IN_GAME)
        return;

    if (RemotePlayer *p = Utils::getLocalPlayer(client.allPlayers))
    {
        if (p->state == RemotePlayerState::GameOver)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
            {
                client.packedSendRejoin();
            }
            return;
        }
        // --- Mouvement ---
        bool left = sf::Keyboard::isKeyPressed(Config::Get().keys.left);
        bool right = sf::Keyboard::isKeyPressed(Config::Get().keys.right);
        bool up = sf::Keyboard::isKeyPressed(Config::Get().keys.up);
        bool down = sf::Keyboard::isKeyPressed(Config::Get().keys.down);

        sf::Vector2f vel{0.f, 0.f};

        if (left && !right)
            vel.x = -Config::Get().speed;
        if (right && !left)
            vel.x = Config::Get().speed;
        if (up && !down)
            vel.y = -Config::Get().speed;
        if (down && !up)
            vel.y = Config::Get().speed;

        // normaliser diagonale
        if (vel.x != 0.f && vel.y != 0.f)
            vel /= std::sqrt(2.f);

        p->velocity = vel;

        // --- Tir ---
        if (sf::Keyboard::isKeyPressed(Config::Get().keys.fire))
        {
            double now = Utils::localTimeNow();
            // double fireInterval = 1.0 / client.localPlayer.fireRate;
            double fireInterval = 1.0 / p->fireRate;

            if (now - p->lastShootTime >= fireInterval)
            {
                client.packetSendBullets(BulletType::LINEAR);
                p->lastShootTime = now;
            }
        }
        // --- Rocket ---
        if (sf::Keyboard::isKeyPressed(Config::Get().keys.rocket))
        {
            double now = Utils::localTimeNow();
            double fireInterval = 1.0 / p->rocketRate;

            if (now - p->lastShootRocketTime >= fireInterval)
            {
                client.packetSendBullets(BulletType::HOMING_MISSILE);
                p->lastShootRocketTime = now;
            }
        }
    };
}
