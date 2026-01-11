#include "NetworkDiscovery.hpp"

void NetworkDiscovery::startBroadcast(uint16_t gamePort, uint16_t discoveryPort)
{
    stopBroadcast();
    broadcasting = true;
    broadcastThread = std::thread([this, gamePort, discoveryPort]()
                                  {
        sf::UdpSocket socket;
        socket.setBlocking(false);
        sf::IpAddress broadcast = sf::IpAddress::Broadcast;
        sf::Packet packet;
        packet << std::string("RTYPE_DISCOVERY:") + std::to_string(gamePort);

        while (broadcasting)
        {
            if (socket.send(packet, broadcast, discoveryPort) != sf::Socket::Status::Done)
                std::cerr << "[DISCOVERY] Broadcast failed\n";

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        } });
}

void NetworkDiscovery::stopBroadcast()
{
    broadcasting = false;
    if (broadcastThread.joinable())
        broadcastThread.join();
}

void NetworkDiscovery::startListening(uint16_t discoveryPort)
{
    stopListening();
    listening = true;
    listenThread = std::thread([this, discoveryPort]()
                               {
        sf::UdpSocket socket;
        if (socket.bind(discoveryPort, sf::IpAddress::Any) != sf::Socket::Status::Done)
        {
            std::cerr << "[DISCOVERY] Failed to bind UDP socket\n";
            return;
        }
        socket.setBlocking(false);

        while (listening)
        {
            sf::Packet packet;
            std::optional<sf::IpAddress> sender = sf::IpAddress::Any;

            unsigned short senderPort = 0;
            if (socket.receive(packet, sender, senderPort) == sf::Socket::Status::Done)
            {
                std::string msg;
                packet >> msg;
                const std::string tag = "RTYPE_DISCOVERY:";
                if (msg.rfind(tag, 0) == 0)
                {
                    uint16_t gamePort = static_cast<uint16_t>(std::stoi(msg.substr(tag.size())));
                    std::lock_guard<std::mutex> lock(mtx);
                    discoveredServers.push_back({sender->toString(), gamePort});
                }
            }
            sf::sleep(sf::milliseconds(20));
        }

        socket.unbind(); });
}

void NetworkDiscovery::stopListening()
{
    listening = false;
    if (listenThread.joinable())
        listenThread.join();
}

std::vector<DiscoveredServer> NetworkDiscovery::getDiscoveredServers()
{
    std::lock_guard<std::mutex> lock(mtx);
    return discoveredServers;
}
