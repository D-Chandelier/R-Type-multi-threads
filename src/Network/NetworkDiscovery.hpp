#pragma once
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <vector>

struct DiscoveredServer
{
    std::string ip;
    uint16_t port;

    bool operator==(const DiscoveredServer &other) const
    {
        return ip == other.ip && port == other.port;
    }
};
class NetworkDiscovery
{
public:
    NetworkDiscovery() = default;
    ~NetworkDiscovery() = default;

    // Broadcast du serveur sur le LAN
    void broadcastServer(uint16_t gamePort, uint16_t discoveryPort = 4445);

    // Scanner le LAN à la recherche de serveurs
    std::vector<DiscoveredServer> scanLAN(uint16_t discoveryPort = 4445, unsigned int timeoutMs = 1000);
};
