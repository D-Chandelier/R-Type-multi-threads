#pragma once
#include <SFML/System/Vector2.hpp>

struct Turret
{
    sf::Vector2f position; // position absolue dans le monde
    bool active;           // si la tourelle est active ou détruite
    float shootCooldown;   // timer pour tirer

    Turret(sf::Vector2f pos)
        : position(pos), active(true), shootCooldown(0.f) {}
};
