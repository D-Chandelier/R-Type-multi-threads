#include "Enemies.hpp"
#include "../Network/Client/Client.hpp"

Enemy::Enemy(sf::Vector2f pos)
    : position(pos) {}

void Enemy::draw(Client &c, sf::RenderWindow &w)
{
    switch (type)
    {
    case EnemyType::TURRET:
        drawTurret(c, w);
        break;

    default:
        break;
    }
}

void Enemy::drawTurret(Client &c, sf::RenderWindow &w)
{
    sf::Sprite sprite(Config::Get().turretTexture);
    int cellX = Config::Get().turretTexture.getSize().x / 6;
    int cellY = Config::Get().turretTexture.getSize().y / 2;
    sprite.setTextureRect(sf::IntRect({cellX * 1, cellY * 0}, {cellX, cellY}));
    sprite.setScale({3.f, 3.f});
    sprite.setOrigin({cellX * 0.5f, cellY * 0.5f});
    float screenX, screenY;

    {
        std::lock_guard<std::mutex> lock(c.enemiesMutex);
        if (!active)
            return;

        screenX = position.x - c.terrain.worldX;
        screenY = position.y;
    }

    sprite.setPosition({screenX, screenY});
    w.draw(sprite);
}

void Enemy::update(float dt, float worldX)
{
    if (!active)
        return;

    switch (type)
    {
    case EnemyType::TURRET:
    {
        updateTurret(dt, worldX);
        break;
    }

    default:
        break;
    }
}

void Enemy::updateTurret(float dt, float worldX)
{
    // Hors écran (à gauche)
    if (position.x < worldX - TURRET_WIDTH)
    {
        active = false;
        return;
    }

    // Cooldown de tir
    shootCooldown -= dt;
    if (shootCooldown <= 0.f)
    {
        shootCooldown += TURRET_FIRE_INTERVAL;
        if (position.x - worldX < Config::Get().windowSize.x)
            wantsToShoot = true;
    }
}
