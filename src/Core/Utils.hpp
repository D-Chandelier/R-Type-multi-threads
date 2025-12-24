#pragma once
#include <SFML/Network.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/String.hpp>
#include <string>
#include "../Entities/RemotePlayers.hpp"
#include "Config.hpp"

// std::string getLocalIpAddress()
// {
//     auto optIp = sf::IpAddress::getLocalAddress();
//     if (!optIp.has_value())
//         return "0.0.0.0"; // ou "" selon ton besoin

//     return optIp->toString();
// }

namespace Utils
{
    inline double localTimeNow()
    {
        using namespace std::chrono;
        return duration<double>(steady_clock::now().time_since_epoch()).count();
    }

    inline double currentGameTime(double start)
    {
        return localTimeNow() - start;
    }

    inline std::string keyToString(sf::Keyboard::Scancode k)
    {
        return sf::Keyboard::getDescription(k).toAnsiString(); // SFML >= 2.6
    }

    inline RemotePlayer *getLocalPlayer(std::map<int, RemotePlayer> &p)
    {
        auto it = p.find(Config::Get().playerId);
        if (it != p.end())
            return &it->second;
        return nullptr;
    }

    inline RemotePlayer *getPlayerByPeer(ENetPeer *peer, std::map<int, RemotePlayer> &allPlayers)
    {
        for (auto &[id, player] : allPlayers)
        {
            if (player.peer == peer)
                return &player;
        }
        return nullptr;
    }
}
