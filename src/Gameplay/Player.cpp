#include "Player.hpp"
#include "RemotePlayers.hpp"
#include <iostream>

Player::Player() : sprite(texture)
{
    if (!texture.loadFromFile("assets/r-typesheet42.gif"))
    {
        std::cerr << "Failed to load assets/r-typesheet42.gif\n";
    }

    int fw = static_cast<int>(texture.getSize().x / 5);
    int fh = static_cast<int>(texture.getSize().y / 5);

    sprite.setTextureRect(sf::IntRect({frameX * fw, frameY * fh}, {fw, fh}));
    sprite.setOrigin({fw / 2.f, fh / 2.f});
    sprite.setScale({2.f, 2.f});
    sprite.setPosition({50.f, 300.f});
}

void Player::setId(int newId)
{
    id = newId;
    frameY = newId;

    int fw = texture.getSize().x / 5;
    int fh = texture.getSize().y / 5;

    sprite.setTextureRect(sf::IntRect({frameX * fw, frameY * fh}, {fw, fh}));
}

void Player::update(float dt, BulletManager &bullets)
{
    if (!client || client->ConnexionState != ClientState::CONNECTED)
        return;

    // --- MOUVEMENT ---
    sf::Vector2f move{0.f, 0.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        move.y -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        move.y += speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        move.x -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        move.x += speed;

    bool shooting = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

    // if (move.x != 0.f || move.y != 0.f)
    ClientInput in;
    in.playerId = id;
    in.dx = move.x;
    in.dy = move.y;
    in.shooting = shooting ? 1 : 0;

    client->sendInput(in);

    // appliquer la position du snapshot serveur
    if (client->playerId != UINT32_MAX && client->remotePlayers.count(client->playerId))
    {
        auto &p = client->remotePlayers[client->playerId];
        sprite.setPosition({p.x, p.y});

        int fw = texture.getSize().x / 5;
        int fh = texture.getSize().y / 5;

        sprite.setTextureRect(sf::IntRect(
            {static_cast<int>(p.frameX) * fw,
             static_cast<int>(p.id) * fh},
            {fw, fh}));
    }
}
void Player::draw(sf::RenderWindow &window) const
{
    window.draw(sprite);
}
