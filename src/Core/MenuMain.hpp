#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>

#include "Config.hpp"

struct MenuStar
{
    sf::Vector2f pos;
    float speed;
    float size;
    sf::Color color;
};
extern std::vector<MenuStar> stars;

enum class MenuResult
{
    NONE,
    PLAY,
    JOIN,
    QUIT
};

class MenuMain
{
public:
    MenuMain();
    MenuResult show(sf::RenderWindow &window);

private:
    MenuResult handleClick(const sf::Vector2f &pos);
    sf::Text title;
    sf::Text play;
    sf::Text join;
    sf::Text quit;
};
