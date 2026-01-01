#include "Server.hpp"

void Server::playerCollision(RemotePlayer &p)
{
    bool blockedX = false;
    bool blockedY = false;

    // 1. Appliquer le mouvement
    p.position += p.velocity * SERVER_TICK;

    sf::FloatRect player = p.getBounds();

    for (const auto &seg : terrain.segments)
    {
        for (const auto &block : seg.blocks)
        {
            TerrainBlock b = block;
            b.rect.position.x += seg.startX - worldX; // ⚠️ ABSOLU, PAS worldX

            if (!b.rect.findIntersection(player))
                continue;

            // Overlaps (MTV)
            float dx1 = b.rect.position.x + b.rect.size.x - player.position.x;
            float dx2 = player.position.x + player.size.x - b.rect.position.x;
            float dy1 = b.rect.position.y + b.rect.size.y - player.position.y;
            float dy2 = player.position.y + player.size.y - b.rect.position.y;

            float overlapX = min(dx1, dx2);
            float overlapY = min(dy1, dy2);

            // Résolution sur l’axe le plus faible
            if (overlapX < overlapY)
            {
                if (player.position.x < b.rect.position.x)
                    p.position.x -= overlapX;
                else
                    p.position.x += overlapX;

                p.velocity.x = 0.f;

                blockedX = true;
            }
            else
            {
                if (player.position.y < b.rect.position.y)
                    p.position.y -= overlapY; // petit fudge pour éviter de rester collé
                else
                    p.position.y += overlapY;

                p.velocity.y = 0.f;
            }
            // Clamp final écran (optionnel)
            p.position.x = std::clamp(
                p.position.x,
                0.f,
                static_cast<float>(Config::Get().windowSize.x) - player.size.x);

            p.position.y = std::clamp(
                p.position.y,
                0.f,
                static_cast<float>(Config::Get().windowSize.y) - player.size.y);

            // Mettre à jour le rect après correction
            player = p.getBounds();
            if (player.findIntersection(b.rect))

                if (blockedX && p.position.x <= 0.f && !p.invulnerable)
                {
                    killAndRespawn(p);
                    return;
                }
        }
    }
    p.position.x = std::clamp(
        p.position.x,
        0.f,
        static_cast<float>(Config::Get().windowSize.x) - player.size.x);

    p.position.y = std::clamp(
        p.position.y,
        0.f,
        static_cast<float>(Config::Get().windowSize.y) - player.size.y);
}
