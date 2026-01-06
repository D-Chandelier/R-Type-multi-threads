#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

constexpr float BACKGROUND_SCROLL_SPEED = 40.f;
constexpr float LEVEL_SCROLL_SPEED = 200.f; // 180.f
constexpr float PLAYER_SCROLL_SPEED = 70.f; // 120
constexpr float FRAMERATE = 120.f;          // 120
constexpr float MAX_PLAYER = 4.f;

struct KeyBindings
{
    sf::Keyboard::Key up = sf::Keyboard::Key::Up;
    sf::Keyboard::Key down = sf::Keyboard::Key::Down;
    sf::Keyboard::Key left = sf::Keyboard::Key::Left;
    sf::Keyboard::Key right = sf::Keyboard::Key::Right;
    sf::Keyboard::Key fire = sf::Keyboard::Key::LControl;
    sf::Keyboard::Key rocket = sf::Keyboard::Key::LAlt;
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

    uint32_t playerId = -1;
    std::string playerName = "001";
    std::string serverIp = "127.0.0.1";
    uint16_t serverPort = 1234;
    uint16_t discoveryPort = serverPort + 1;
    // uint16_t maxPlayers = 4;
    bool isServer = false;
    float speed = PLAYER_SCROLL_SPEED;
    uint32_t frameRate = FRAMERATE; // FPS / Hz

    sf::Texture playerTexture;
    sf::Texture blockTexture;
    sf::Texture turretTexture;
    sf::Texture rocketTexture;
    sf::Texture bckgTextureBack, bckgTextureFront;

    sf::Texture bonusRocketTex;
    sf::Texture bonusHealthTex;
    sf::Texture bonusShieldTex;
    sf::Texture bonusFireRateTex;
    sf::Texture bonusScoreTex;

    sf::Texture RocketX3Tex;
    sf::Texture HealthX1Tex;
    sf::Texture FireRateUpTex;

    sf::Texture ExplosionTex;

    sf::Vector2f playerScale = {2.f, 2.f};
    sf::FloatRect playerArea;

    sf::FloatRect gameArea;

private:
    Config()
    {
        if (!font.openFromFile("assets/kenvector_future_thin.ttf"))
            std::cerr << "[CONFIG] Impossible de charger la font\n";
        if (!playerTexture.loadFromFile("assets/r-typesheet42.gif"))
            std::cerr << "[CONFIG] Impossible de charger r-typesheet42.gif\n";
        if (!blockTexture.loadFromFile("assets/world_01_192x192.png"))
            std::cerr << "[CONFIG] Impossible de charger world_01_192x192.png\n";
        if (!turretTexture.loadFromFile("assets/turret_02_96x32.png"))
            std::cerr << "[CONFIG] Impossible de charger turret_02_96x32.png\n";
        if (!rocketTexture.loadFromFile("assets/Rocket.png"))
            std::cerr << "[CONFIG] Impossible de charger Rocket.png\n";
        if (!bckgTextureBack.loadFromFile("assets/Blue_Nebula_08-1024x1024.png"))
            std::cerr << "[CONFIG] Impossible de charger Starfield_07-1024x1024.png\n";
        if (!bckgTextureFront.loadFromFile("assets/Starfield_07-1024x1024.png"))
            std::cerr << "[CONFIG] Impossible de charger Blue_Nebula_08-1024x1024.png\n";
        if (!RocketX3Tex.loadFromFile("assets/RocketX3.png"))
            std::cerr << "[CONFIG] Impossible de charger RocketX3.png\n";
        if (!HealthX1Tex.loadFromFile("assets/HealthX1.png"))
            std::cerr << "[CONFIG] Impossible de charger HealthX1.png\n";
        if (FireRateUpTex.loadFromFile("assets/FireRateUp.png"))
            std::cerr << "[CONFIG] Impossible de charger FireRateUp.png\n";
        if (ExplosionTex.loadFromFile("assets/Explosion.png"))
            std::cerr << "[CONFIG] Impossible de charger Explosion.png\n";

        bckgTextureBack.setRepeated(true);
        bckgTextureFront.setRepeated(true);

        playerArea = sf::FloatRect{{0.f, 0.f}, {static_cast<float>(playerTexture.getSize().x / 5), static_cast<float>(playerTexture.getSize().y / 5)}};
        gameArea = sf::FloatRect{{0.f, 0.f}, {static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)}};
    }
};
