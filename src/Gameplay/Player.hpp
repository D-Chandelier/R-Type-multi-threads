#pragma once
#include <SFML/Graphics.hpp>
#include "BulletManager.hpp"
#include "../Network/Client.hpp"
#include "../Core/Config.hpp"

class Player
{
public:
    Player();

    void update(float dt, BulletManager &bullets);
    void draw(sf::RenderWindow &window) const;
    void setId(int newId);

    sf::Sprite &getSprite() { return sprite; }
    sf::Texture &getTexture() { return texture; }
    sf::Vector2f getPosition() { return sprite.getPosition(); }

    void setBulletTexture(sf::Texture *tex) { bulletTexture = tex; };

    int hp = 5; // point de vie
    // Tir
    bool shooting = false; // vrai si tir en cours

    void setClient(Client *c) { client = c; }
    Client *client = nullptr;

    int id = Config::Get().id;
    int frameY = 0; // ligne 0 = joueur 1

private:
    sf::Texture texture;
    sf::Sprite sprite;

    float speed = 300.f;
    sf::Vector2f screenSize;

    // Animation du vaisseau (sprite)
    int frameX = 2; // colonne centrale = neutre
    // int frameY = 0; // ligne 0 = joueur 1

    float shootTimer = 0.f;     // temps écoulé depuis dernier tir
    float shootInterval = 0.2f; // tir toutes les 0.2 secondes

    enum class ShootLevel
    {
        Level1,
        Level2,
        Level3
    };
    ShootLevel shootLevel = ShootLevel::Level3;
    sf::Texture *bulletTexture = nullptr; // pointer vers texture chargée dans Game ou Player

    // Client client;
    // Config config;
    std::string playerName = Config::Get().playerName;
};
