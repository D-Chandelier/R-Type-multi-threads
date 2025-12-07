#pragma once
#include <SFML/Graphics.hpp>
#include "../Gameplay/Player.hpp"
#include "../Gameplay/RemotePlayers.hpp"
#include "../Gameplay/BulletManager.hpp"
#include "../Gameplay/EnemyManager.hpp"
#include "../Network/Server.hpp"
#include "../Network/Client.hpp"
#include "GameState.hpp"
#include "Menu.hpp"

class Game
{
public:
    Game();
    void run();

private:
    void update(float dt);
    void render();
    // void shootPlayer(); // fonction pour gérer le tir du joueur

    sf::Clock clock;
    Server server;
    Client client;
    Menu menu;

    sf::RenderWindow window;
    Player player;
    BulletManager bullets;
    EnemyManager enemies;

    float enemySpawnTimer = 0.f;
    float spawnInterval = 2.0f; // spawn toutes les 2 sec
    int maxEnemiesOnScreen = 5; // limite d'ennemis actifs

    sf::Texture playerBulletTex;

    GameState state = GameState::MENU_MAIN;
};
