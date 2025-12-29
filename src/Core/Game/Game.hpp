#pragma once
#include <SFML/Graphics.hpp>

#include "../../UI/Menus/IMenu.hpp"
#include "../../UI/Menus/MenuMain.hpp"
#include "../../UI/Menus/MenuServersList.hpp"
#include "../../UI/Menus/MenuOption.hpp"
#include "../../UI/Menus/MenuInGame.hpp"
#include "GameState.hpp"

#include "../../Network/Client/Client.hpp"
#include "../../Network/Server/Server.hpp"
#include "../../Network/Protocols/NetworkDiscovery.hpp"
#include "../../World/Terrain.hpp"

#include "../../Entities/Bullet.hpp"

#include "../Utils.hpp"

class Game
{
public:
    Game();
    void run();
    void runServer(); //(Server *server);
    void runClient();
    void stopThreads();

    void update(float dt);
    void updateGameplay(float dt);
    void updateBackgrounds();
    void updatePlayers();
    void updateBullets(float dt);

    void draw(float dt);
    void drawGameplay();
    void drawBackground();
    void drawTerrain();
    void drawPlayers();
    void drawBullets();
    void drawTurrets();
    void drawPlayersHUD();
    void drawGameOverUI();

    void handleEvent();
    void handleMenuAction();
    void handleEventPlayerMove(float dt);

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

    std::atomic<bool> running{true};

    GameState state = GameState::MENU_MAIN;

    // sf::Texture backgroundTexture1, backgroundTexture2;
    sf::VertexArray backgroundVA_1, backgroundVA_2, playersVA, bulletsVA;

    double background_1_OffsetX = 0.0;
    double background_1_OffsetY = 0.0;
    double background_2_OffsetX = 0.0;
    double background_2_OffsetY = 0.0;
};
