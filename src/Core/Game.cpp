#include "Game.hpp"

std::atomic<bool> running{true};

void Game::drawDebug()
{
    // Dessin du joueur
    RemotePlayer *p = getLocalPlayer(client.allPlayers);
    if (p)
    {
        sf::RectangleShape playerRect;
        sf::FloatRect bounds = p->getBounds();
        playerRect.setPosition({bounds.position.x, bounds.position.y});
        playerRect.setSize({bounds.size.x, bounds.size.y});
        playerRect.setFillColor(sf::Color(255, 0, 0, 128)); // rouge semi-transparent
        window.draw(playerRect);
    }

    // Dessin des blocs
    for (const auto &seg : client.terrain.segments)
    {
        for (const auto &block : seg.blocks)
        {
            sf::FloatRect blockRect = block.rect;
            blockRect.position.x += seg.startX - client.terrain.worldX; // coordonnées absolues

            sf::RectangleShape rect;
            rect.setPosition({blockRect.position.x, blockRect.position.y});
            rect.setSize({blockRect.size.x, blockRect.size.y});
            // Couleur selon solidité
            rect.setFillColor(sf::Color(255, 0, 0, 128));
            window.draw(rect);
        }
    }
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

    bulletsVA.setPrimitiveType(sf::PrimitiveType::Triangles);

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

void runServer(Server *server)
{
    using clock = std::chrono::steady_clock;
    auto last = clock::now();

    while (running)
    {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        dt = std::min(dt, 0.1f);
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

    const float targetFrameTime = 1.f / Config::Get().frameRate;

    while (running)
    {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        dt = std::min(dt, 0.1f); // sécurité

        client->update(dt);

        float sleepTime = targetFrameTime - dt;
        if (sleepTime > 0.f)
        {
            std::this_thread::sleep_for(std::chrono::duration<float>(targetFrameTime));
        }
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

void Game::onPlayerMove(float dt)
{
    if (state != GameState::IN_GAME)
        return;

    if (RemotePlayer *p = getLocalPlayer(client.allPlayers))
    {
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
            double now = localTimeNow();
            // double fireInterval = 1.0 / client.localPlayer.fireRate;
            double fireInterval = 1.0 / p->fireRate;

            if (now - p->lastShootTime >= fireInterval)
            {
                client.sendBullets();
                p->lastShootTime = now;
            }
        }
    };
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

        while (!server.serverReady)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

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

void Game::run()
{
    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        // 1. Gestion des events (juste fermeture, menus...)
        handleEvent();

        // 2. Calcul mouvement/tir joueur chaque frame
        onPlayerMove(dt);

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

void Game::updateGameplay(float dt)
{
    updateBackgrounds();
    // updateTerrain();
    updatePlayers();
    updateBullets(dt);
}

void Game::updateBackgrounds()
{

    float texW = static_cast<float>(backgroundTexture1.getSize().x);
    float texH = static_cast<float>(backgroundTexture1.getSize().y);
    sf::Vector2f winSize = {static_cast<float>(Config::Get().windowSize.x), static_cast<float>(Config::Get().windowSize.y)};

    double gameTime = localTimeNow() + client.serverTimeOffset;

    background_1_OffsetX = std::fmod(
        static_cast<float>(gameTime) * client.backgroundScrollSpeed,
        texH);
    background_2_OffsetX = std::fmod(
        static_cast<float>(gameTime) * client.backgroundScrollSpeed * 2.f,
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

void Game::updateTerrain()
{
    // Interpolation pour smooth scrolling
    double delta = localTimeNow() - client.serverGameTime;
    float alpha = static_cast<float>(delta / (1.0f / Config::Get().frameRate));
    alpha = std::clamp(alpha, 0.f, 1.f);

    client.terrain.worldX += (client.targetWorldX - client.terrain.worldX) * alpha;

    // Nettoyage segments déjà passés (optionnel)
    while (!client.terrain.segments.empty() &&
           client.terrain.segments.front().startX + SEGMENT_WIDTH < client.terrain.worldX - 100.f)
    {
        client.terrain.segments.pop_front();
    }
}

void Game::updatePlayers()
{
    int i = 0;
    int cellWidth, cellHeight;

    playersVA.resize(client.allPlayers.size() * 6);

    for (auto &[id, p] : client.allPlayers)
    {
        bool visible = true;
        if (p.invulnerable)
        {
            double t = localTimeNow() - p.respawnTime;
            visible = static_cast<int>(t * 10) % 2 == 0; // 10 Hz
        }
        if (!visible)
            continue; // ne pas dessiner ce frame

        // Temps écoulé depuis la dernière position serveur
        double delta = localTimeNow() - p.lastUpdateTime;
        // Interpolation simple
        float alpha = static_cast<float>(delta / (2.f / Config::Get().frameRate)); // delta / tick serveur (xx ms)
        alpha = std::clamp(alpha, 0.f, 1.f);

        // p.position = p.serverPosition; // reset avant lerp
        p.position += (p.serverPosition - p.position) * alpha;

        cellWidth = Config::Get().playerArea.size.x;
        cellHeight = Config::Get().playerArea.size.y;

        sf::Vector2f origin = p.position; // getOrigine();
        sf::Vector2f scale = Config::Get().playerScale;
        float w = p.getBounds().size.x; // static_cast<float>(cellWidth) * scale.x;
        float h = p.getBounds().size.y; // static_cast<float>(cellHeight) * scale.y;

        float x = p.getBounds().position.x; // p.position.x; //(p.position.x - origin.x * scale.x);
        float y = p.getBounds().position.y; //(p.position.y - origin.y * scale.y);

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

        i++;
    }
}

void Game::updateBullets(float dt)
{
    // Mise à jour logique
    for (auto &[id, b] : client.allBullets)
    {
        b.update(dt);
    }

    // Nettoyage
    std::erase_if(client.allBullets,
                  [](const auto &it)
                  { return !it.second.active; });

    rebuildBulletsVA();
}

void Game::rebuildBulletsVA()
{
    bulletsVA.clear();
    bulletsVA.setPrimitiveType(sf::PrimitiveType::Triangles);

    for (const auto &[id, b] : client.allBullets)
    {
        float w = 10.f;
        float h = 5.f;
        float x = b.position.x - w / 2.f;
        float y = b.position.y - h / 2.f;

        sf::Color color = sf::Color::Cyan;

        bulletsVA.append({{x, y}, color});
        bulletsVA.append({{x + w, y}, color});
        bulletsVA.append({{x + w, y + h}, color});
        bulletsVA.append({{x, y}, color});
        bulletsVA.append({{x + w, y + h}, color});
        bulletsVA.append({{x, y + h}, color});
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
        drawGameplay();

    window.display();
}

void Game::drawGameplay()
{
    drawBackground();
    drawTerrain();
    drawPlayers();
    drawBullets();
    // drawDebug(); // debug
}

void Game::drawBackground()
{
    sf::RenderStates states_1, states_2;
    states_1.texture = &backgroundTexture1;
    states_2.texture = &backgroundTexture2;

    window.draw(backgroundVA_1, states_1);
    window.draw(backgroundVA_2, states_2);
}

void Game::drawTerrain()
{
    sf::Sprite sprite(Config::Get().blockTexture);
    // sprite.setTexture(blocksTexture);

    for (const auto &seg : client.terrain.segments)
    {
        for (const auto &b : seg.blocks)
        {
            float screenX = b.rect.position.x + seg.startX - client.terrain.worldX;

            if (screenX + b.rect.size.x < 0 ||
                screenX > Config::Get().windowSize.x)
                continue;

            sprite.setPosition({screenX, b.rect.position.y});
            sprite.setTextureRect(client.terrain.getTextureRect(b.visual));

            window.draw(sprite);
        }
    }
    // for (const auto &seg : client.terrain.segments)
    // {
    //     for (const auto &block : seg.blocks)
    //     {
    //         sf::FloatRect blockRect = block;
    //         blockRect.position.x += seg.startX - client.terrain.worldX; // coordonnées absolues

    //         sf::RectangleShape rect;
    //         rect.setPosition({blockRect.position.x, blockRect.position.y});
    //         rect.setSize({blockRect.size.x, blockRect.size.y});
    //         // Couleur selon solidité
    //         rect.setFillColor(sf::Color(61, 43, 31));
    //         window.draw(rect);
    //     }
    // }
}

void Game::drawPlayers()
{
    sf::RenderStates states;
    states.texture = &Config::Get().texture;
    window.draw(playersVA, states);
}

void Game::drawBullets()
{
    if (bulletsVA.getVertexCount() == 0)
        return;

    sf::RenderStates states;
    states.texture = nullptr; //&Config::Get().texture; // même texture que tes sprites joueurs, ou autre texture pour bullets
    window.draw(bulletsVA, states);
}
