#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Config
{
public:
    static Config &Get()
    {
        static Config instance;
        return instance;
    }

    // empêche copie/assignation
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

public:
    // ====== PARAMÈTRES MODIFIABLES ======
    std::string title = "R-Type";
    sf::Vector2u windowSize{1280, 800};
    uint32_t id = 0;
    std::string playerName = "001";
    std::string serverIp = "127.0.0.1";
    uint16_t serverPort = 1234;
    uint16_t maxPlayers = 4;
    bool isServer = false;
    float speed = 300.f;

private:
    Config() = default; // constructeur privé → Singleton
};
