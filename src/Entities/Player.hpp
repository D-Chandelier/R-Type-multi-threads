#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

// #include "../Network/Client.hpp"
class Client; // <<-- forward declaration

class Player
{
public:
    Player();
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Vector2f position;
    sf::Vector2f velocity;
    int id;

    void sendPosition(Client &client);
};
