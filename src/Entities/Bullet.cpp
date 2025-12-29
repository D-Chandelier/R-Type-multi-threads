#include "Bullet.hpp"

void Bullet::update(float dt)
{
    position += velocity * dt;

    // Exemple destruction hors écran
    if (position.x < -50 || position.x > Config::Get().windowSize.x + 50 ||
        position.y < -50 || position.y > Config::Get().windowSize.y + 50)
    {
        active = false;
    }
}