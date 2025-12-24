#include "Server.hpp"

void Server::packetReceivedPlayerPosition(ENetEvent event, float dt)
{
    if (event.packet->dataLength != sizeof(ClientPositionPacket))
        return;
    auto *p = reinterpret_cast<ClientPositionPacket *>(event.packet->data);
    auto it = allPlayers.find(p->id);
    if (it == allPlayers.end())
        return;
    it->second.velocity = sf::Vector2f(p->velX, p->velY);
    checkPlayerCollision(it->second);
    it->second.position.x += p->velX * SERVER_TICK; // SERVER_TICK = dt côté serveur
    it->second.position.y += p->velY * SERVER_TICK;
}

void Server::packetReceivedBulletShoot(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ClientBulletPacket))
        return;

    auto *p = reinterpret_cast<ClientBulletPacket *>(event.packet->data);

    RemotePlayer *shooter = Utils::getPlayerByPeer(event.peer, allPlayers);
    if (!shooter)
        return;

    sf::Vector2f dir{p->velX, p->velY};
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0.f)
        return;

    dir /= len;

    ServerBullet bullet;
    bullet.id = nextBulletId++;
    bullet.position = sf::Vector2f(shooter->getBounds().getCenter()); // sf::Vector2f{shooter->serverX, shooter->serverY}; // PAS p->x/p->y
    bullet.velocity = dir * shooter->bulletSpeed;
    bullet.damage = shooter->bulletDamage;
    bullet.ownerId = shooter->id;

    allBullets.emplace(bullet.id, bullet);
    packetBroadcastBullets(bullet);
}