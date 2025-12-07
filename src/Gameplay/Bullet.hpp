#pragma once
#include <SFML/Graphics.hpp>

enum class BulletType {
    Player,
    Enemy
};

struct Bullet {
    sf::Vector2f pos;
    sf::Vector2f velocity;
    sf::Texture* texture = nullptr;
    sf::Vector2f size = {54.f, 9.f}; // taille logique pour hitbox
    BulletType type;

    Bullet(const sf::Vector2f& start, const sf::Vector2f& vel,
           sf::Texture* tex, const sf::Vector2f& s, BulletType t)
        : pos(start), velocity(vel), texture(tex), size(s), type(t) 
        {}

    void update(float dt) {
        pos += velocity * dt;
    }

    sf::FloatRect getBounds() const {
        return sf::FloatRect(pos, size);
    }

    void draw(sf::RenderWindow& window) {
        if (type == BulletType::Enemy) {
            // tir ennemi simple : rectangle rouge
            sf::RectangleShape rect;
            rect.setSize({12.f, 2.f});
            rect.setOrigin({size.x / 2, size.y / 2}); // centrer
            rect.setPosition(pos);
            rect.setFillColor(sf::Color(255, 80, 80));
            window.draw(rect); 
        } else {
            if (!texture) return;
            sf::Sprite sprite(*texture);
            sprite.setPosition(pos);
            sprite.setScale({size.x / texture->getSize().x, size.y / texture->getSize().y});
            window.draw(sprite);
        }
    }
};
