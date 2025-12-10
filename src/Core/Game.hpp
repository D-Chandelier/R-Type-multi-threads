#pragma once
#include <SFML/Graphics.hpp>

#include "IMenu.hpp"
#include "MenuMain.hpp"
#include "MenuServersList.hpp"
#include "GameState.hpp"

#include "../Network/Client.hpp"
#include "../Network/Server.hpp"

class Game
{
public:
    Game();
    void run();
    void stopThreads();
    void drawGameplay(sf::RenderWindow &w);
    void handleMenuAction();
    void updateGameplay(float dt);

private:
    sf::RenderWindow window;

    MenuMain menuMain;
    MenuServersList menuServers;
    IMenu *currentMenu = nullptr;

    Server server;
    Client client;

    std::thread serverThread;
    std::thread clientThread;
    GameState state = GameState::MENU_MAIN;
};

// #pragma once
// #include <SFML/Graphics.hpp>
// #include <thread>
// #include <chrono>
// #include <atomic>
// #include <iostream>

// #include "../Core/MenuServersList.hpp"
// #include "../Core/MenuMain.hpp"

// #include "../Network/Server.hpp"
// #include "../Network/NetworkDiscovery.hpp"
// #include "../Network/Server.hpp"
// #include "../Network/Client.hpp"

// #include "GameState.hpp"
// #include "Config.hpp"

// class Game
// {
// public:
//     Game();
//     void run();

// private:
//     void update(float dt);
//     void render();
//     // void shootPlayer(); // fonction pour gérer le tir du joueur

//     sf::Clock clock;
//     Server server;
//     Client client;
//     MenuMain menuMain;
//     MenuServersList menuServersList;

//     sf::RenderWindow window;
//     std::thread serverThread;
//     std::thread clientThread;

//     float enemySpawnTimer = 0.f;
//     float spawnInterval = 2.0f; // spawn toutes les 2 sec
//     int maxEnemiesOnScreen = 5; // limite d'ennemis actifs

//     sf::Texture playerBulletTex;

//     GameState state = GameState::MENU_MAIN;
// };
