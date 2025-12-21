#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

constexpr float BACKGROUND_SCROLL_SPEED = 20.f;

struct KeyBindings
{
    sf::Keyboard::Key up = sf::Keyboard::Key::Up; // sf::Keyboard::Key::Up;
    sf::Keyboard::Key down = sf::Keyboard::Key::Down;
    sf::Keyboard::Key left = sf::Keyboard::Key::Left;
    sf::Keyboard::Key right = sf::Keyboard::Key::Right;
    sf::Keyboard::Key fire = sf::Keyboard::Key::LControl;
};

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
    sf::Color backgroundColor = sf::Color(30, 30, 30);
    sf::Color fontColor = sf::Color::White;
    sf::Color hoverColor = sf::Color(255, 200, 0); // Jaune
    sf::Font font;

    KeyBindings keys;

    int playerId = -1;
    std::string playerName = "001";
    std::string serverIp = "127.0.0.1";
    uint16_t serverPort = 1234;
    uint16_t discoveryPort = serverPort + 1;
    uint16_t maxPlayers = 4;
    bool isServer = false;
    float speed = 300.f;
    uint32_t frameRate = 120; // FPS / Hz

    sf::Texture texture;
    sf::Vector2f playerScale = {2.f, 2.f};
    sf::FloatRect playerArea;

    sf::FloatRect gameArea;

private:
    Config()
    {
        if (!font.openFromFile("assets/kenvector_future_thin.ttf"))
            std::cerr << "[CONFIG] Impossible de charger la font\n";
        if (!texture.loadFromFile("assets/r-typesheet42.gif"))
            std::cerr << "[CONFIG] Impossible de charger r-typesheet42.gif\n";

        playerArea = sf::FloatRect{{0.f, 0.f}, {static_cast<float>(texture.getSize().x / 5), static_cast<float>(texture.getSize().y / 5)}};
        gameArea = sf::FloatRect{{0.f, 0.f}, {static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)}};
    }
};
