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

#include "../Entities/EntityManager.hpp"
#include "../Entities/Player.hpp"

class Game
{
public:
    Game();
    void run();
    void stopThreads();
    void drawGameplay(sf::RenderWindow &w);
    void update(float dt);
    void draw(float dt);
    void handleMenuAction();
    void updateGameplay(float dt);

    // Player player;

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

    EntityManager entityManager;
};
