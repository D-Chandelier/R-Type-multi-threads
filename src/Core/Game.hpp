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
    MenuOption menuOption;
    MenuInGame menuInGame;
    IMenu *currentMenu = nullptr;

    Server server;
    Client client;

    std::thread serverThread;
    std::thread clientThread;
    NetworkDiscovery discoveryClient;

    GameState state = GameState::MENU_MAIN;
};
