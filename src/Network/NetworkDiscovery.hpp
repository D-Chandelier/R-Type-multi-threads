#pragma once
#include <SFML/Network.hpp>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

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
    NetworkDiscovery() : listening(false) {}
    void startBroadcast(uint16_t gamePort, uint16_t discoveryPort);
    void stopBroadcast();
    void startListening(uint16_t discoveryPort);
    void stopListening();
    std::vector<DiscoveredServer> getDiscoveredServers();

private:
    std::atomic<bool> broadcasting{false};
    std::atomic<bool> listening{false};
    std::thread broadcastThread;
    std::thread listenThread;

    std::vector<DiscoveredServer> discoveredServers;
    std::mutex mtx;
};
