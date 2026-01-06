#pragma once
#include <SFML/System/Vector2.hpp>
#include "../Core/Config.hpp"

class Client;

struct Explosion
{
    sf::Vector2f position;
    float time = 0.f;
    float duration = 0.5f;

    sf::Sprite sprite;
    int frame = 0;
    int frameCount = 10;     // nombre total de frames
    float frameTime = 0.06f; // durée d’une frame (25 fps)
    float timer = 0.f;

    int frameWidth;
    int frameHeight;

    bool finished = false;

    Explosion(const sf::Texture &tex, sf::Vector2f pos)
        : position(pos), sprite(tex)
    {
        frameWidth = tex.getSize().x / frameCount;
        frameHeight = tex.getSize().y;

        sprite.setOrigin(
            {frameWidth * 0.5f,
             frameHeight * 0.5f});

        sprite.setPosition(pos);
        sprite.setTextureRect(sf::IntRect({0, 0}, {frameWidth, frameHeight}));
    }

    static void spawnExplosion(Client &client, sf::Vector2f pos);
    static void updateExplosionsClient(Client &client, float dt);
    static void draw(Client &client, sf::RenderWindow &w);
};