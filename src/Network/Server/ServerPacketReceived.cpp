#include "Server.hpp"

void Server::packetReceivedPlayerPosition(ENetEvent event, float dt)
{
    if (event.packet->dataLength != sizeof(ClientPositionPacket))
        return;

    auto *p = reinterpret_cast<ClientPositionPacket *>(event.packet->data);
    auto it = allPlayers.find(p->id);
    if (it == allPlayers.end())
        return;

    RemotePlayer &player = it->second;

    // 1) Mettre à jour la vélocité
    player.velocity = sf::Vector2f(p->velX, p->velY);

    // 2) Mettre à jour la position serveur
    player.position += player.velocity * dt;

    // 3) Vérifier collisions
    playerCollision(player);
}

void Server::packetReceivedBulletShoot(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ClientBulletPacket))
        return;

    auto *p = reinterpret_cast<ClientBulletPacket *>(event.packet->data);

    RemotePlayer *shooter = Utils::getPlayerByPeer(event.peer, allPlayers);
    if (!shooter)
        return;

    // Utiliser la position envoyée par le client
    sf::Vector2f spawnPos(p->x, p->y);

    sf::Vector2f dir{p->velX, p->velY};
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0.f)
        return;

    dir /= len;
    if (p->bulletType == BulletType::HOMING_MISSILE)
    {
        if (shooter->nbRocket <= 0)
            return;
        shooter->nbRocket--;
    }

    Bullet bullet;
    bullet.id = nextBulletId++;
    bullet.position = sf::Vector2f(shooter->getBounds().getCenter());
    bullet.position.y += p->bulletType == BulletType::HOMING_MISSILE ? shooter->getBounds().size.y / 2 : 0;
    bullet.velocity = dir * BULLET_SPEED;
    bullet.type = p->bulletType;
    bullet.damage = bullet.type == BulletType::LINEAR ? shooter->bulletDamage : shooter->rocketDamage;
    bullet.ownerId = shooter->id;

    allBullets.emplace(bullet.id, bullet);
    packetBroadcastBullets(bullet);
}

void Server::packetReceivedRejoin(ENetEvent event)
{
    if (event.packet->dataLength != sizeof(ClientRejoinPacket))
        return;

    RemotePlayer *p = Utils::getPlayerByPeer(event.peer, allPlayers);
    if (!p)
        return;

    p->score = 0.f;
    p->pv = 3.f;
    p->alive = true;
    p->invulnerable = true;

    p->position =
        {
            Config::Get().playerArea.size.x * 0.5f,
            Config::Get().windowSize.y / 9.f *
                (Utils::getPlayerByPeer(p->peer, allPlayers)->id + 1.f)};

    p->velocity = {0.f, 0.f};

    const double now = Utils::currentGameTime(gameStartTime);
    p->respawnTime = now + 3.0; // invulnérabilité
}