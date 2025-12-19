
#pragma once
#include "Bullet.hpp"

class BulletManager
{
    std::vector<Bullet> bullets;

public:
    BulletManager(size_t maxBullets) { bullets.reserve(maxBullets); }

    void shoot(const Vector2 &pos, const Vector2 &vel, float dmg, BulletType type, int ownerId)
    {
        for (auto &b : bullets)
        {
            if (!b.active)
            {
                b.position = pos;
                b.velocity = vel;
                b.damage = dmg;
                b.type = type;
                b.ownerId = ownerId;
                b.active = true;
                return;
            }
        }
        bullets.emplace_back(pos, vel, dmg, type, ownerId);
    }

    void update(float dt)
    {
        for (auto &b : bullets)
            if (b.active)
                b.update(dt);
    }

    std::vector<Bullet *> getActiveBullets()
    {
        std::vector<Bullet *> active;
        for (auto &b : bullets)
            if (b.active)
                active.push_back(&b);
        return active;
    }
};
