#pragma once
#include <SFML/Graphics.hpp>
struct Player
{
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Vector2f position;
    sf::Vector2f velocity;
    int id;

    Player() : position(0.f, 0.f), velocity(0.f, 0.f), id(-1), sprite(texture)
    {
        sprite.setTexture(Config::Get().texture);

        // Taille d'une cellule
        int cellWidth = sprite.getTexture().getSize().x / 5;
        int cellHeight = sprite.getTexture().getSize().y / 5;
        sprite.setTextureRect(sf::IntRect({2 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}));
        sprite.setScale({2.f, 2.f});
        sprite.setOrigin(sprite.getLocalBounds().getCenter());
    };
};
