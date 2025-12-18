#pragma once
#include <SFML/Graphics.hpp>
// #include <iostream>

// #include "../Network/Client.hpp"
// class Client; // <<-- forward declaration

struct Player
{
    // public:
    // Player();
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Vector2f position;
    sf::Vector2f velocity;
    int id;
    // void sendPosition(Client &client);

    Player() : position(0.f, 0.f), velocity(0.f, 0.f), id(-1), sprite(texture)
    {
        if (!texture.loadFromFile("assets/r-typesheet42.gif"))
            std::cout << "Erreur loading r-typesheet42.gif" << "\n";
        sprite.setTexture(texture);

        // Taille d'une cellule
        int cellWidth = sprite.getTexture().getSize().x / 5;
        int cellHeight = sprite.getTexture().getSize().y / 5;
        sprite.setTextureRect(sf::IntRect(
            {2 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}));
        sprite.setScale({2.f, 2.f});
        sprite.setOrigin(
            sprite.getLocalBounds().getCenter());
        };
};
