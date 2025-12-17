#include "Game.hpp"

std::atomic<bool> running{true};

// Threads réseau
void runServer(Server *server)
{
    using clock = std::chrono::steady_clock;
    auto last = clock::now();

    while (running)
    {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        server->update(dt);

        // petit sleep pour ne pas saturer le CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void runClient(Client *client)
{
    using clock = std::chrono::steady_clock;
    auto last = clock::now();

    while (running)
    {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        client->update(dt);
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(Config::Get().frameRate)));
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

Game::Game()
    : window(sf::VideoMode(Config::Get().windowSize),
             Config::Get().title),
      menuMain(),
      menuServers(client, discoveryClient),
      menuOption(),
      menuInGame(),
      menuBackground(Config::Get().font)
{
    currentMenu = &menuMain;
}

void Game::run()
{
    sf::Clock clock;
    // Background bg(Config::Get().font);

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        // 1. Gestion des events (juste fermeture, menus...)
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

        // 2. Calcul mouvement joueur chaque frame
        if (state == GameState::IN_GAME)
        {
            client.localPlayer.velocity = {0.f, 0.f};
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
                client.localPlayer.velocity.x -= Config::Get().speed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
                client.localPlayer.velocity.x += Config::Get().speed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
                client.localPlayer.velocity.y -= Config::Get().speed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
                client.localPlayer.velocity.y += Config::Get().speed;

            // normaliser la diagonale si besoin
            if (client.localPlayer.velocity.x != 0.f && client.localPlayer.velocity.y != 0.f)
                client.localPlayer.velocity /= std::sqrt(2.f);

            if (client.localPlayer.velocity != sf::Vector2f(0, 0))
            {
                client.localPlayer.position += client.localPlayer.velocity * dt;
                if (client.localPlayer.position.x < 0)
                    client.localPlayer.position.x = 0;
                if (client.localPlayer.position.x > Config::Get().windowSize.x)
                    client.localPlayer.position.x = Config::Get().windowSize.x;
                if (client.localPlayer.position.y < 0)
                    client.localPlayer.position.y = 0;
                if (client.localPlayer.position.y > Config::Get().windowSize.y)
                    client.localPlayer.position.y = Config::Get().windowSize.y;
                client.localPlayer.sendPosition(client);
            }
        }
        update(dt);
        draw(dt);
    }
}

void Game::update(float dt)
{
    if (currentMenu && state != GameState::IN_GAME)
    {
        currentMenu->update(dt, window);
        menuBackground.update(dt);
        handleMenuAction(); // <-- important
    }
    else if (state == GameState::IN_GAME)
    {
        updateGameplay(dt); // <-- ta future logique in-game
    }
}

void Game::draw(float dt)
{
    window.clear(Config::Get().backgroundColor);

    if (state != GameState::IN_GAME)
    {
        menuBackground.draw(window);
        currentMenu->draw(window);
    }
    else
        drawGameplay(window);

    window.display();
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

        // --- Démarrage serveur ---
        if (!server.start(Config::Get().serverPort))
        {
            std::cerr << "[Game] Impossible de démarrer le serveur.\n";
            break;
        }
        serverThread = std::thread(runServer, &server);

        // Petit délai pour s'assurer que le serveur est prêt avant connexion client
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // --- Démarrage client local ---
        if (!client.init())
        {
            std::cerr << "[Game] Impossible d'initialiser le client.\n";
            break;
        }

        if (!client.connectTo("127.0.0.1", Config::Get().serverPort))
        {
            std::cerr << "[Game] Impossible de connecter le client.\n";
            break;
        }

        clientThread = std::thread(runClient, &client);

        discoveryClient.startBroadcast(Config::Get().serverPort, Config::Get().discoveryPort);

        state = GameState::IN_GAME;
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

void Game::updateGameplay(float dt) {}

void Game::drawGameplay(sf::RenderWindow &w)
{
    int cellWidth = client.localPlayer.sprite.getTexture().getSize().x / 5;
    int cellHeight = client.localPlayer.sprite.getTexture().getSize().y / 5;

    for (const auto &[id, p] : client.allPlayers)
    {
        sf::Sprite tempSprite(Config::Get().texture);
        tempSprite.setTextureRect(sf::IntRect(
            {2 * cellWidth, id * cellHeight}, {cellWidth, cellHeight}));
        tempSprite.setScale({2.f, 2.f});
        tempSprite.setOrigin(tempSprite.getLocalBounds().getCenter());
        tempSprite.setPosition({p.x, p.y});
        w.draw(tempSprite);
    }
}
