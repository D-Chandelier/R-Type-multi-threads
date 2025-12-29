#include "Client.hpp"

void Client::packetSendPosition()
{
    // envoyer la position au serveur
    if (Config::Get().playerId < 0 || Config::Get().playerId > Config::Get().maxPlayers)
        return;

    if (peer)
    {

        // Clamp pour ne pas sortir de l'écran
        float halfW = Config::Get().playerArea.getCenter().x * Config::Get().playerScale.x;
        float halfH = Config::Get().playerArea.getCenter().y * Config::Get().playerScale.y;

        RemotePlayer *localPlayer = Utils::getLocalPlayer(allPlayers);
        if (localPlayer)
        {
            ClientPositionPacket p;
            p.header.type = static_cast<uint8_t>(PacketType::CLIENT_MSG);
            p.header.code = static_cast<uint8_t>(ClientMsg::PLAYER_POSITION);
            p.id = localPlayer->id;
            p.velX = localPlayer->velocity.x;
            p.velY = localPlayer->velocity.y;
            ENetPacket *packet = enet_packet_create(&p, sizeof(p), 0);
            enet_peer_send(peer, 0, packet);
        }
    }
}

void Client::packetSendBullets()
{
    // envoyer la position au serveur
    if (Config::Get().playerId < 0 || Config::Get().playerId > Config::Get().maxPlayers)
        return;

    if (peer)
    {
        ClientBulletPacket p;
        p.header.type = static_cast<uint8_t>(PacketType::CLIENT_MSG);
        p.header.code = static_cast<uint8_t>(ClientMsg::BULLET_SHOOT);
        p.ownerId = Config::Get().playerId;
        p.x = Utils::getLocalPlayer(allPlayers)->getBounds().getCenter().x;
        p.y = Utils::getLocalPlayer(allPlayers)->getBounds().getCenter().y;
        p.velX = 1.f;
        p.velY = 0.f;

        ENetPacket *packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE); // Un tir ne doit jamais être perdu.
        enet_peer_send(peer, 0, packet);
    }
}

void Client::packedSendRejoin()
{
    if (Config::Get().playerId < 0 || Config::Get().playerId > Config::Get().maxPlayers)
        return;

    if (peer)
    {
        ClientRejoinPacket p;
        p.header.type = static_cast<uint8_t>(PacketType::CLIENT_MSG);
        p.header.code = static_cast<uint8_t>(ClientMsg::REJOIN);
        p.id = Config::Get().playerId;

        ENetPacket *packet = enet_packet_create(&p, sizeof(p), ENET_PACKET_FLAG_RELIABLE); // Un tir ne doit jamais être perdu.
        enet_peer_send(peer, 0, packet);
    }
}