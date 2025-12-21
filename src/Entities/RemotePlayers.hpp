#pragma once
#include <iostream>
#include <enet/enet.h>

struct RemotePlayer
{
    int id;

    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f size;

    sf::Vector2f serverPosition;

    float bulletSpeed = 500.f;
    float bulletDamage = 10.f;
    float lerpFactor = 0.3f; // fluidité du mouvement, 0 = pas de mouvement, 1 = téléport
    double lastShootTime = 0.0;
    float fireRate = 4.f; // tirs par seconde

    bool alive = true;
    bool invulnerable = false;
    double respawnTime = 0.0;

    double lastUpdateTime = 0.0;

    ENetPeer *peer = nullptr;

    sf::FloatRect getBounds() const
    {
        return {
            {position.x, position.y},
            {Config::Get().playerArea.size.x * Config::Get().playerScale.x,
             Config::Get().playerArea.size.y * Config::Get().playerScale.y}};
    }
    // sf::Vector2f getOrigine() const
    // {
    //     return {
    //         Config::Get().playerArea.size.x * Config::Get().playerScale.x * 0.5f,
    //         Config::Get().playerArea.size.y * Config::Get().playerScale.y * 0.5f};
    // }
};