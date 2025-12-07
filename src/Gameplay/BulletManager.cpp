#include "BulletManager.hpp"
#include <algorithm>

void BulletManager::shoot(const sf::Vector2f& start, const sf::Vector2f& velocity,
                          sf::Texture* texture, const sf::Vector2f& size, BulletType type) {
    bullets.emplace_back(start, velocity, texture, size, type);
}

void BulletManager::update(float dt) {
    for (auto& b : bullets)
        b.update(dt);

    // supprimer les bullets hors écran
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const Bullet& b){ return b.pos.x > 2000.f || b.pos.x < -50.f; }),
        bullets.end());
}

void BulletManager::draw(sf::RenderWindow& window) {
    for (auto& b : bullets)
        b.draw(window);
}
