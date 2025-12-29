#pragma once
#include <cstdint>
#include <SFML/System/Vector2.hpp>

constexpr float TURRET_HEIGHT = 48.f;
constexpr float TURRET_WIDTH = 48.f;
constexpr float TURRET_FIRE_INTERVAL = 1.0f;

class Terrain;
struct Turret
{
    uint32_t id;
    sf::Vector2f position;
    sf::Vector2f velocity;
    uint32_t pv = 2;
    bool active = true;
    float points = 10.f;
    float shootCooldown = 0.f; // timer pour tirer

    Turret(sf::Vector2f pos)
        : position(pos) {}
};
