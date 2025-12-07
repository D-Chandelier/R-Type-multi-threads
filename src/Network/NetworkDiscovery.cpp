#include "NetworkDiscovery.hpp"
#include <sstream>

void NetworkDiscovery::broadcastServer(uint16_t gamePort, uint16_t discoveryPort)
{
    sf::UdpSocket socket;

    // Créer le packet à envoyer
    sf::Packet packet;
    packet << std::string("RTYPE_DISCOVERY:") + std::to_string(gamePort);

    sf::IpAddress broadcast = sf::IpAddress::Broadcast;

    if (socket.send(packet, broadcast, discoveryPort) != sf::Socket::Status::Done)
    {
        std::cerr << "[DISCOVERY] Broadcast failed\n";
    }
}

std::vector<DiscoveredServer> NetworkDiscovery::scanLAN(uint16_t discoveryPort, unsigned int timeoutMs)
{
    std::vector<DiscoveredServer> results;
    sf::UdpSocket socket;

    // bind sur le port de découverte
    if (socket.bind(discoveryPort) != sf::Socket::Status::Done)
    {
        std::cerr << "[DISCOVERY] Failed to bind UDP socket\n";
        return results;
    }

    socket.setBlocking(false);

    sf::Packet packet;
    std::optional<sf::IpAddress> sender;
    unsigned short senderPort = 0;

    sf::Clock timer;
    while (timer.getElapsedTime().asMilliseconds() < static_cast<int>(timeoutMs))
    {
        auto status = socket.receive(packet, sender, senderPort);
        if (status == sf::Socket::Status::Done && sender.has_value())
        {
            std::string msg;
            packet >> msg; // extraire le texte depuis le Packet

            const std::string tag = "RTYPE_DISCOVERY:";
            if (msg.rfind(tag, 0) == 0)
            {
                uint16_t gamePort = static_cast<uint16_t>(std::stoi(msg.substr(tag.size())));
                results.push_back({sender->toString(), gamePort});
                // std::cout << "[DISCOVERY] Found server " << sender->toString() << ":" << gamePort << "\n";
            }

            packet.clear(); // prêt pour la prochaine réception
        }
        sf::sleep(sf::milliseconds(20));
    }

    socket.unbind();
    return results;
}
