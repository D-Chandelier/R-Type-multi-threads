#pragma once
#include <SFML/Graphics.hpp>

#include "IMenu.hpp"
#include "MenuMain.hpp"
#include "MenuServersList.hpp"
#include "MenuOption.hpp"
#include "MenuInGame.hpp"
#include "GameState.hpp"

#include "../Network/Client.hpp"
#include "../Network/Server.hpp"
#include "../Network/NetworkDiscovery.hpp"
#include "Utils.hpp"

class Game
{
public:
    Game();
    void run();
    void stopThreads();
    void drawGameplay(sf::RenderWindow &w);
    void update(float dt);
    void draw(float dt);
    void handleEvent();
    void handleMenuAction();
    void onPlayerMove(float dt);
    void updateGameplay(float dt);
    void updateBackgrounds();
    void updatePlayers();
    void updateBullets(float dt);
    void rebuildBulletsVA();
    void drawBackground();
    void drawPlayers();
    void drawBullets();

private:
    sf::RenderWindow window;

    MenuMain menuMain;
    MenuServersList menuServers;
    MenuOption menuOption;
    MenuInGame menuInGame;
    IMenu *currentMenu = nullptr;
    Background menuBackground;

    Server server;
    Client client;

    std::thread serverThread;
    std::thread clientThread;
    NetworkDiscovery discoveryClient;

    GameState state = GameState::MENU_MAIN;

    sf::Texture backgroundTexture1, backgroundTexture2;
    sf::VertexArray backgroundVA_1, backgroundVA_2, playersVA, bulletsVA;

    double background_1_OffsetX = 0.0;
    double background_1_OffsetY = 0.0;
    double background_2_OffsetX = 0.0;
    double background_2_OffsetY = 0.0;

    // std::vector<Bullet> receivedBullets;
};
