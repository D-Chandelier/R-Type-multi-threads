#include "RemotePlayers.hpp"
#include "../Core/Utils.hpp"
#include "../Network/Server/Server.hpp"

void RemotePlayer::killAndRespawn(RemotePlayer &p, Server &s)
{
    p.pv--;
    p.alive = false;
    p.invulnerable = true;

    p.position =
        {
            Config::Get().playerArea.size.x * 0.5f,
            Config::Get().windowSize.y / 9.f * (Utils::getPlayerByPeer(p.peer, s.allPlayers)->id + 1.f)};

    p.velocity = {0.f, 0.f};

    const double now = Utils::currentGameTime(s.gameStartTime);
    p.respawnTime = now + 3.0; // timestamp absolu
}