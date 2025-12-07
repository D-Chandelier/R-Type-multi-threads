#pragma once
#include <cstdint>
#include <SFML/Graphics.hpp>
#include <map>

struct RemotePlayer
{
    uint32_t id{};
    float x{};
    float y{};
    uint32_t frameX{};

    void draw(sf::RenderWindow &window, const sf::Texture &texture)
    {
        sf::Sprite sprite(texture);
        sprite.setPosition({x, y});
        int fw = texture.getSize().x / 5;
        int fh = texture.getSize().y / 5;

        sprite.setTextureRect(sf::IntRect(sf::Vector2i(frameX * fw, id * fh), {fw, fh}));
        sprite.setScale({2.f, 2.f});
        window.draw(sprite);
    }
};
