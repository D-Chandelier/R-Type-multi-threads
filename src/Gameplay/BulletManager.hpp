#pragma once
#include "Bullet.hpp"
#include <vector>

class BulletManager {
public:
    void shoot(const sf::Vector2f& start, const sf::Vector2f& velocity,
               sf::Texture* texture, const sf::Vector2f& size, BulletType type);

    void update(float dt);
    void draw(sf::RenderWindow& window);

    const std::vector<Bullet>& getBullets() const { return bullets; }
    std::vector<Bullet>& accessBullets() { return bullets; }

private:
    std::vector<Bullet> bullets;
};
