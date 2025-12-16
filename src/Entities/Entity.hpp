// Entity.hpp
#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>

enum class EntityType
{
    PLAYER,
    BULLET,
    ENEMY
};

struct Entity
{
    uint32_t id;
    EntityType type;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float rotation = 0.f;
    bool alive = true;

    sf::RectangleShape shape; // placeholder visuel simple

    Entity(uint32_t _id, EntityType _type, sf::Vector2f pos)
        : id(_id), type(_type), position(pos)
    {
        shape.setPosition(pos);
        switch (type)
        {
        case EntityType::PLAYER:
            shape.setSize({40.f, 20.f});
            shape.setFillColor(sf::Color::Green);
            break;
        case EntityType::BULLET:
            shape.setSize({10.f, 4.f});
            shape.setFillColor(sf::Color::Yellow);
            break;
        case EntityType::ENEMY:
            shape.setSize({30.f, 15.f});
            shape.setFillColor(sf::Color::Red);
            break;
        }
    }

    void update(float dt)
    {
        position += velocity * dt;
        shape.setPosition(position);
    }

    void draw(sf::RenderWindow &w)
    {
        w.draw(shape);
    }
};
