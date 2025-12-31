#pragma once

#include <map>
#include <string>
#include <chrono>

#include <SFML/Window/Keyboard.hpp>
#include <enet/enet.h>

#include "Config.hpp"
#include "../Entities/RemotePlayers.hpp"

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

    inline RemotePlayer *getLocalPlayer(std::map<uint32_t, RemotePlayer> &p)
    {
        auto it = p.find(Config::Get().playerId);
        if (it != p.end())
            return &it->second;
        return nullptr;
    }

    inline RemotePlayer *getPlayerByPeer(ENetPeer *peer, std::map<uint32_t, RemotePlayer> &allPlayers)
    {
        for (auto &[id, player] : allPlayers)
        {
            if (player.peer == peer)
                return &player;
        }
        return nullptr;
    }

    inline uint32_t findFreePlayerId(std::map<uint32_t, RemotePlayer> &allPlayers)
    {
        for (uint32_t id = 0; id < MAX_PLAYER; ++id)
        {
            if (allPlayers.find(id) == allPlayers.end())
                return id;
        }
        return 100; // aucun slot libre
    }
}
