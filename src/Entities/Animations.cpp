#include "Animations.hpp"
#include "../Network/Client/Client.hpp"

void Explosion::updateExplosionsClient(Client &client, float dt)
{
    for (auto it = client.explosions.begin(); it != client.explosions.end();)
    {
        Explosion &e = *it;

        e.time += dt;
        e.timer += dt;

        if (e.timer >= e.frameTime)
        {
            e.timer -= e.frameTime;
            e.frame++;

            if (e.frame >= e.frameCount)
            {
                it = client.explosions.erase(it);
                continue;
            }

            e.sprite.setTextureRect(sf::IntRect(
                {e.frame * e.frameWidth, 0},
                {e.frameWidth, e.frameHeight}));
        }

        ++it;
    }
}

void Explosion::spawnExplosion(Client &client, sf::Vector2f pos)
{
    client.explosions.emplace_back(
        Config::Get().ExplosionTex,
        pos);
}

void Explosion::draw(Client &client, sf::RenderWindow &w)
{
    for (auto &e : client.explosions)
    {
        e.sprite.setPosition({e.position.x - client.targetWorldX,
                              e.position.y});

        w.draw(e.sprite);
    }
}
