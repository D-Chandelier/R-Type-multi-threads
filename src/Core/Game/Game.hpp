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
    void runServer();
    void runClient();
    void stopThreads();

    void update(float dt);
    void updateGameplay(float dt);
    void updateBackgrounds();
    void updatePlayers();

    void draw(float dt);
    void drawGameplay();
    void drawBackground();
    void drawTerrain();
    void drawPlayers();
    void drawEnemies();
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

    sf::VertexArray backgroundVA_1, backgroundVA_2, playersVA;

    double background_1_OffsetX = 0.0;
    double background_1_OffsetY = 0.0;
    double background_2_OffsetX = 0.0;
    double background_2_OffsetY = 0.0;
};
