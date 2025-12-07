#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Config.hpp"

constexpr int MENU_MAIN_PLAY = 0;
constexpr int MENU_MAIN_JOIN = 1;
constexpr int MENU_MAIN_OPTIONS = 2;
constexpr int MENU_MAIN_QUIT = 3;

constexpr int MENU_PAUSE_RESUME = 0;
constexpr int MENU_PAUSE_MAINMENU = 1;
constexpr int MENU_PAUSE_QUIT = 2;

enum ActiveField
{
    NONE = -1,
    FIELD_NAME = 0,
    FIELD_IP = 1,
    FIELD_PORT = 2
};

class Menu
{
public:
    Menu();

    // Affiche un menu principal classique
    int drawMainMenu(sf::RenderWindow &window);

    // Menu en jeu (resume / menu / quitte)
    int drawPauseMenu(sf::RenderWindow &window);

    // Menu des options (retour)
    int drawOptionsMenu(sf::RenderWindow &window, Config &config);

private:
    // Config config;
    sf::Font font;
    sf::Text makeItem(const std::string &str, float y);

    ActiveField activeField = NONE;
    sf::Clock cursorClock;
};
