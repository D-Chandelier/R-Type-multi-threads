// #include "Player.hpp"
// #include "../Network/Client.hpp"

// Player::Player()
//     : sprite(texture)
// {
//     if (!texture.loadFromFile("assets/r-typesheet42.gif"))
//         std::cout << "Erreur loading r-typesheet42.gif" << "\n";

//     sprite.setTexture(texture);

//     // Taille d'une cellule
//     int cellWidth = sprite.getTexture().getSize().x / 5;
//     int cellHeight = sprite.getTexture().getSize().y / 5;
//     sprite.setTextureRect(sf::IntRect(
//         {2 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}));
// }

// void Player::sendPosition(Client &client)
// {
//     // envoyer la position au serveur
//     if (client.localPlayer.id > Config::Get().maxPlayers)
//         return;
//     if (client.peer)
//     {
//         client.localPlayer.position.x = std::clamp(
//             client.localPlayer.position.x,
//             client.localPlayer.sprite.getLocalBounds().getCenter().x * 2,
//             Config::Get().windowSize.x - sprite.getLocalBounds().getCenter().x * 2);
//         client.localPlayer.position.y = std::clamp(
//             client.localPlayer.position.y,
//             client.localPlayer.sprite.getLocalBounds().getCenter().y * 2,
//             Config::Get().windowSize.y - sprite.getLocalBounds().getCenter().y * 2);

//         ClientPositionPacket p;
//         p.header.type = static_cast<uint8_t>(PacketType::CLIENT_MSG);
//         p.header.code = static_cast<uint8_t>(ClientMsg::PLAYER_POSITION);
//         p.id = client.localPlayer.id;
//         p.x = client.localPlayer.position.x;
//         p.y = client.localPlayer.position.y;
//         // std::cout
//         //     << "[PLAYER] sendPosition(PLAYER_POSITION):\n"
//         //     << "Id: " << p.id << "\n"
//         //     << "Poition > x: " << p.x << ", y: " << p.y << "\n";

//         ENetPacket *packet = enet_packet_create(&p, sizeof(p), 0);
//         enet_peer_send(client.peer, 0, packet);
//         // enet_host_flush(client.clientHost);
//     }
// }
