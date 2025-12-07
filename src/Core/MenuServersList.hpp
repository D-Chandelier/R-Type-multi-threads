#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <set>

#include "../Network/NetworkDiscovery.hpp"
#include "../Network/Client.hpp"

#include "Config.hpp"
#include "Game.hpp"
#include "GameState.hpp"
#include "MenuMain.hpp"

struct MenuServerStar
{
    sf::Vector2f pos;
    float speed;
    float size;
    sf::Color color;
};
extern std::vector<MenuServerStar> serverStars;

class MenuServersList
{

public:
    MenuServersList();
    MenuResult showServerList(sf::RenderWindow &window, Client &client, uint16_t discoveryPort = 5000);

private:
    sf::Text title, ssTitle, quit;
};