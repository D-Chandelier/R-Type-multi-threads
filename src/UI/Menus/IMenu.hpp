#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include "../../Core/Config.hpp"

enum class MenuAction
{
    NONE,
    QUIT_APP,
    GO_TO_MAIN_MENU,
    GO_TO_SERVER_LIST,
    GO_TO_OPTION_MENU,
    START_GAME,
    JOIN_SERVER,
    RETURN_IN_GAME
};

struct Star
{
    sf::Vector2f pos;
    float speed;
    float size;
    sf::Color color;
};

class Background
{
    std::vector<Star> stars;
    sf::Text title;
    float timeAccumulator = 0.f;

public:
    Background(const sf::Font &font)
        : title(font)
    {
        init();
        title.setString(Config::Get().title);
        title.setCharacterSize(80);
        title.setOrigin(title.getLocalBounds().getCenter());
    }

    void init()
    {
        title.setPosition({Config::Get().windowSize.x / 2.f,
                           Config::Get().windowSize.y / 10.f});
        stars.reserve(50);
        for (int i = 0; i < 50; i++)
        {
            stars.push_back({{float(rand() % Config::Get().windowSize.x), float(rand() % Config::Get().windowSize.y)},
                             30.f + float(rand() % 70),
                             1.f + float(rand() % 3),
                             sf::Color(150 + rand() % 105, 150 + rand() % 105, 150 + rand() % 105)});
        }
    }

    void update(float dt)
    {
        timeAccumulator += dt;

        for (auto &s : stars)
        {
            s.pos.x -= s.speed * dt;
            if (s.pos.x < -5)
                s.pos.x = Config::Get().windowSize.x + 5;

            int delta = (rand() % 15) - 7;
            int r = std::clamp<int>(s.color.r + delta, 150, 255);
            int g = std::clamp<int>(s.color.g + delta, 150, 255);
            int b = std::clamp<int>(s.color.b + delta, 150, 255);
            s.color = sf::Color(r, g, b);
        }

        // Animation titre
        float hue = std::fmod(timeAccumulator * 200.f, 360.f);
        auto hsvToRgb = [](float h, float s, float v) -> sf::Color
        {
            float c = v * s;
            float x = c * (1 - std::fabs(std::fmod(h / 60.f, 2) - 1));
            float m = v - c;
            float r = 0, g = 0, b_ = 0;
            if (h < 60)
            {
                r = c;
                g = x;
                b_ = 0;
            }
            else if (h < 120)
            {
                r = x;
                g = c;
                b_ = 0;
            }
            else if (h < 180)
            {
                r = 0;
                g = c;
                b_ = x;
            }
            else if (h < 240)
            {
                r = 0;
                g = x;
                b_ = c;
            }
            else if (h < 300)
            {
                r = x;
                g = 0;
                b_ = c;
            }
            else
            {
                r = c;
                g = 0;
                b_ = x;
            }
            return sf::Color(uint8_t((r + m) * 255), uint8_t((g + m) * 255), uint8_t((b_ + m) * 255));
        };
        title.setFillColor(hsvToRgb(hue, 1.f, 1.f));
        float rotation = std::sin(timeAccumulator * 2.f) * 2.f;
        title.setRotation(sf::degrees(rotation));
    }

    void draw(sf::RenderWindow &w)
    {
        sf::RectangleShape px;
        for (auto &s : stars)
        {
            px.setSize({s.size, s.size});
            px.setPosition(s.pos);
            px.setFillColor(s.color);
            w.draw(px);
        }
        w.draw(title);
    }
};

class IMenu
{
public:
    virtual ~IMenu() = default;

    void init();
    virtual void update(float dt, sf::RenderWindow &w) = 0;
    virtual void handleEvent(const sf::Event &, sf::RenderWindow &) = 0;
    virtual void draw(sf::RenderWindow &) = 0;

    // Quand un menu veut quitter / changer d’écran
    virtual MenuAction getAction() const = 0;

    // Pour réinitialiser l’état du menu quand on y retourne
    virtual void reset() {};
};
