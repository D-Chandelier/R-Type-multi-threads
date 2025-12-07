#include "Game.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <cmath>
#include <algorithm>

Game::Game()
    : window(sf::VideoMode{Config::Get().windowSize, 32u}, "R-Type"), player()
{
    if (!playerBulletTex.loadFromFile("assets/laserBlue01.png"))
        std::cerr << "Erreur chargement laserBlue01.png\n";

    // Passe le pointeur à Player
    player.setBulletTexture(&playerBulletTex);
    player.setClient(&client); // ← donne le vrai client au joueur
    state = GameState::MENU_MAIN;
}

void Game::run()
{
    while (window.isOpen()) // Boucle principale
    {
        float dt = clock.restart().asSeconds();

        // --- Gestion des événements ---
        while (auto eventOpt = window.pollEvent())
        {
            auto &event = *eventOpt;

            // Fermeture de la fenêtre
            if (auto *e = event.getIf<sf::Event::Closed>())
            {
                client.stop();
                server.stop();
                window.close();
            }

            // Touche Échap
            if (auto *e = event.getIf<sf::Event::KeyPressed>())
            {
                if (e->code == sf::Keyboard::Key::Escape)
                {
                    switch (state)
                    {
                    case GameState::PLAYING:
                        state = GameState::MENU_IN_GAME;
                        break;
                    case GameState::MENU_IN_GAME:
                        state = GameState::PLAYING;
                        break;
                    case GameState::MENU_OPTIONS:
                        state = GameState::MENU_MAIN;
                        break;
                    default:
                        break;
                    }
                }
            }

            // TODO : Ici tu peux ajouter d'autres events (clic souris, texte, etc.)
        }

        // --- Mise à jour selon l'état ---
        switch (state)
        {
        case GameState::MENU_MAIN:
        {
            int choice = menu.drawMainMenu(window);

            if (choice == 0) // PLAY
            {
                Config::Get().isServer = true;
                server.start(Config::Get().serverPort);
                client.start();
                client.connectTo("127.0.0.1", Config::Get().serverPort);
                state = GameState::PLAYING;
            }
            else if (choice == 1) // JOIN
            {
                Config::Get().isServer = false;
                client.start();
                client.connectTo(Config::Get().serverIp.c_str(), Config::Get().serverPort);
                state = GameState::PLAYING;
            }
            else if (choice == 2) // OPTIONS
            {
                state = GameState::MENU_OPTIONS;
            }
            else if (choice == 3) // QUIT
            {
                window.close();
            }
        }
        break;

        case GameState::MENU_OPTIONS:
        {
            if (menu.drawOptionsMenu(window, Config::Get()) == 0) // back
                state = GameState::MENU_MAIN;
        }
        break;

        case GameState::MENU_IN_GAME:
        {
            switch (menu.drawPauseMenu(window))
            {
            case 0: // Resume
                state = GameState::PLAYING;
                break;
            case 1: // back to main menu
                client.disconnect();
                server.stop();
                state = GameState::MENU_MAIN;
                break;
            case 2: // quit
                client.disconnect();
                server.stop();
                window.close();
                break;
            }
        }
        break;

        case GameState::PLAYING:
        {
            // float dt = clock.restart().asSeconds();
            // update(dt);
            // render();
        }
        break;
        }

        // --- Update serveur et client TOUJOURS ---
        server.update(dt); // traite connexions et snapshots
        client.update(dt); // traite connexion, packets, déconnexion

        // --- Jouer si connecté ---
        if (state == GameState::PLAYING &&
            (Config::Get().isServer || client.ConnexionState == ClientState::CONNECTED))
        {
            update(dt);
            render();
        }
    }
}

void Game::update(float dt)
{

    if (state == GameState::PLAYING)
    {
        player.update(dt, bullets);
        enemies.update(dt, player.getSprite().getPosition(), bullets);
        bullets.update(dt);

        // ------ Spawn ennemis régulier avec limite ------
        enemySpawnTimer += dt;
        if (enemySpawnTimer >= spawnInterval && enemies.accessEnemies().size() < maxEnemiesOnScreen)
        {
            float y = 50.f + std::rand() % (window.getSize().y - 100);
            enemies.spawnRandom(y); // spawn aléatoire parmi les types disponibles
            enemySpawnTimer = 0.f;
        }

        // Collisions bullets ↔ ennemis
        auto &allBullets = bullets.accessBullets();
        auto &allEnemies = enemies.accessEnemies();

        for (auto &b : allBullets)
        {
            if (b.type != BulletType::Enemy)
            {
                for (auto &e : allEnemies)
                {
                    if (sf::FloatRect(b.pos, {5.f, 5.f}).findIntersection(e.getTransformedBounds()))
                    {
                        // --- CALCULER hitDir AVANT de modifier b.pos ---
                        // Si b.pos est le coin haut-gauche, calcule le centre
                        sf::Vector2f bCenter = b.pos + sf::Vector2f(2.5f, 2.5f); // 5x5 bullet
                        sf::Vector2f eCenter = e.pos;                            // pos est le centre dans ton code

                        sf::Vector2f hitDir = bCenter - eCenter;
                        // inversion Y pour SFML (Y positive vers le bas)
                        // hitDir.y = -hitDir.y;
                        float len = std::sqrt(hitDir.x * hitDir.x + hitDir.y * hitDir.y);
                        if (len > 0.f)
                            hitDir /= len;

                        // Appliquer dégâts
                        e.hp--;

                        // Marquer la bullet pour suppression (après le calcul !)
                        b.pos.x = 2000.f;

                        // si ennemi encore vivant, activer le bouclier visuel
                        if (e.hp > 0)
                        {
                            e.hitDir = hitDir;      // store normalized hit direction
                            e.activateShield(0.5f); // active et set timer
                        }
                        else
                        {
                            // mort -> explosion, score, etc.
                        }
                    }
                }
            }
            if (b.type == BulletType::Enemy)
            {
                sf::FloatRect bulletRect(b.pos, {b.size.x, b.size.y}); // ou {5.f,5.f} si tu veux
                if (bulletRect.findIntersection(player.getSprite().getGlobalBounds()))
                {
                    // --- calcul hitDir pour effets si nécessaire ---
                    sf::Vector2f bCenter = b.pos + b.size * 0.5f; // centre de la bullet
                    sf::Vector2f hitDir = bCenter - player.getPosition();
                    float len = std::sqrt(hitDir.x * hitDir.x + hitDir.y * hitDir.y);
                    if (len > 0.f)
                        hitDir /= len;

                    // appliquer dégâts au joueur
                    player.hp--;

                    // activer un effet de hit sur le joueur (par ex. flash ou bouclier)
                    // playerHitDir = hitDir;
                    // playerActivateHitEffect(0.3f);

                    // supprimer la bullet
                    b.pos.x = 2000.f; // hors écran -> sera supprimée à la prochaine update
                }
            }
        }

        // supprimer bullets mortes
        allBullets.erase(
            std::remove_if(allBullets.begin(), allBullets.end(),
                           [](auto &b)
                           { return b.pos.x > 2000.f; }),
            allBullets.end());

        // Collision joueur ↔ ennemis
        sf::FloatRect playerBounds = player.getSprite().getGlobalBounds();
        for (auto &e : allEnemies)
        {
            if (playerBounds.findIntersection(e.getTransformedBounds()))
            {
                std::cout << "Collision joueur - ennemi !" << std::endl;
                player.hp--;
                e.hp = 0; // mort immédiate
            }
        }

        if (player.hp <= 0)
        {
            player.getSprite().setPosition({50.f, 300.f});
            player.hp = 5;
        }
    }
}

void Game::render()
{
    if (state == GameState::PLAYING)
    {
        window.clear(sf::Color::Black);
        player.draw(window);

        // Dessiner les autres joueurs distants
        for (auto &[id, rp] : client.remotePlayers)
        {
            if (id == client.playerId)
                continue; // on a déjà dessiné le joueur local
            rp.draw(window, player.getTexture());
        }

        enemies.draw(window);
        bullets.draw(window);

        window.display();
    }
}
