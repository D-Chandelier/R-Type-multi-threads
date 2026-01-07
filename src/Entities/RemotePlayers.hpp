#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <iostream>
#include <enet/enet.h>
#include "../Core/Config.hpp"

class Server;

static const std::array<sf::Color, 4> PlayerColors =
    {
        sf::Color(70, 130, 255), // Joueur 1 : bleu
        sf::Color(160, 90, 200), // Joueur 2 : violet
        sf::Color(90, 200, 120), // Joueur 3 : vert
        sf::Color(220, 80, 80)   // Joueur 4 : rouge
};

enum class RemotePlayerState
{
    Playing,
    GameOver
};
struct RemotePlayer
{
    uint32_t id;
    RemotePlayerState state = RemotePlayerState::Playing;
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f size;

    sf::Vector2f serverPosition;

    double lastShootTime = 0.0;
    double lastShootRocketTime = 0.0;
    float fireRate = 4.f; // tirs par seconde
    float rocketRate = 1.f;
    float bulletDamage = 1.f;
    float rocketDamage = 20.f;
    int nbRocket = 3;

    float lerpFactor = 0.3f; // fluidité du mouvement, 0 = pas de mouvement, 1 = téléport

    float score = 0.f;

    bool alive = true;
    float pv = 5.f;
    float maxPv = 5.f;
    bool invulnerable = false;
    float invulnTimer = 0.f;
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

    static void killAndRespawn(RemotePlayer &p, Server &s);
};