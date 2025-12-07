#include "../Core/MenuMain.hpp"
#include "MenuMain.hpp"
#include <cmath>

std::vector<MenuStar> stars;

MenuMain::MenuMain()
    : title(Config::Get().font),
      play(Config::Get().font),
      join(Config::Get().font),
      quit(Config::Get().font)
{
    // --- Texte ---
    title.setString(Config::Get().title);
    title.setCharacterSize(80);
    title.setFillColor(Config::Get().fontColor);
    title.setOrigin({title.getGlobalBounds().size.x / 2, title.getGlobalBounds().size.y / 2});
    title.setPosition({static_cast<float>(Config::Get().windowSize.x / 2), static_cast<float>(Config::Get().windowSize.y / 8)});

    auto setupOption = [&](sf::Text &t, const std::string &str, float yMul)
    {
        t.setString(str);
        t.setCharacterSize(50);
        t.setFillColor(Config::Get().fontColor);
        t.setOrigin({t.getGlobalBounds().size.x / 2, t.getGlobalBounds().size.y / 2});
        t.setPosition({static_cast<float>(Config::Get().windowSize.x / 2), static_cast<float>(Config::Get().windowSize.y / 8 * yMul)});
    };

    setupOption(play, "Play", 3.f);
    setupOption(join, "Join", 4.f);
    setupOption(quit, "Quit", 6.f);

    // --- Génération des étoiles ---
    stars.reserve(50);
    for (int i = 0; i < 50; ++i)
    {
        MenuStar s;
        s.pos = {static_cast<float>(rand() % Config::Get().windowSize.x),
                 static_cast<float>(rand() % Config::Get().windowSize.y)};
        s.speed = 30.f + static_cast<float>(rand() % 70);
        s.size = 1.f + static_cast<float>(rand() % 3);
        s.color = sf::Color(150 + rand() % 105, 150 + rand() % 105, 150 + rand() % 105);
        stars.push_back(s);
    }
}

MenuResult MenuMain::show(sf::RenderWindow &window)
{
    sf::Clock clock;
    float timeAccumulator = 0.f;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        timeAccumulator += dt;

        sf::Vector2f mp(sf::Mouse::getPosition(window));

        // --- Événements ---
        while (auto eventOpt = window.pollEvent())
        {
            auto &event = *eventOpt;

            if (auto *e = event.getIf<sf::Event::Closed>())
                return MenuResult::QUIT;

            if (auto *e = event.getIf<sf::Event::MouseButtonPressed>())
                return handleClick(mp);
        }

        // --- Hover ---
        auto hover = [&](sf::Text &t)
        {
            if (t.getGlobalBounds().contains(mp))
                t.setFillColor(sf::Color::Yellow);
            else
                t.setFillColor(Config::Get().fontColor);
        };
        hover(play);
        hover(join);
        hover(quit);

        // --- Mise à jour étoiles ---
        for (auto &s : stars)
        {
            s.pos.x -= s.speed * dt;
            if (s.pos.x < -5)
                s.pos.x = static_cast<float>(Config::Get().windowSize.x + 5);

            int delta = (rand() % 15) - 7;
            int r = std::clamp<int>(s.color.r + delta, 150, 255);
            int g = std::clamp<int>(s.color.g + delta, 150, 255);
            int b = std::clamp<int>(s.color.b + delta, 150, 255);
            s.color = sf::Color(r, g, b);
        }

        // --- Animation titre ---
        // --- Animation couleur titre ---
        float hue = std::fmod(timeAccumulator * 200.f, 360.f); // vitesse du cycle
        auto hsvToRgb = [](float h, float s, float v) -> sf::Color
        {
            float c = v * s;
            float x = c * (1 - std::fabs(std::fmod(h / 60.f, 2) - 1));
            float m = v - c;
            float r = 0, g = 0, b = 0;

            if (h < 60)
            {
                r = c;
                g = x;
                b = 0;
            }
            else if (h < 120)
            {
                r = x;
                g = c;
                b = 0;
            }
            else if (h < 180)
            {
                r = 0;
                g = c;
                b = x;
            }
            else if (h < 240)
            {
                r = 0;
                g = x;
                b = c;
            }
            else if (h < 300)
            {
                r = x;
                g = 0;
                b = c;
            }
            else
            {
                r = c;
                g = 0;
                b = x;
            }

            return sf::Color(
                static_cast<std::uint8_t>((r + m) * 255),
                static_cast<std::uint8_t>((g + m) * 255),
                static_cast<std::uint8_t>((b + m) * 255));
        };

        title.setFillColor(hsvToRgb(hue, 1.f, 1.f));
        // Oscilation
        float yOffset = std::sin(timeAccumulator * 2.f) * 10.f; // oscillation verticale
        float rotation = std::sin(timeAccumulator * 2.f) * 2.f; // petite rotation
        title.setPosition({static_cast<float>(Config::Get().windowSize.x / 2),
                           static_cast<float>(Config::Get().windowSize.y / 8) + yOffset});
        title.setRotation(sf::degrees(rotation));

        // --- Dessin ---
        window.clear(Config::Get().backgroundColor);

        sf::RectangleShape pixel;
        for (auto &s : stars)
        {
            pixel.setSize({s.size, s.size});
            pixel.setPosition(s.pos);
            pixel.setFillColor(s.color);
            window.draw(pixel);
        }

        window.draw(title);
        window.draw(play);
        window.draw(join);
        window.draw(quit);
        window.display();
    }

    return MenuResult::QUIT;
}

MenuResult MenuMain::handleClick(const sf::Vector2f &mp)
{
    if (play.getGlobalBounds().contains(mp))
        return MenuResult::PLAY;
    if (join.getGlobalBounds().contains(mp))
        return MenuResult::JOIN;
    if (quit.getGlobalBounds().contains(mp))
        return MenuResult::QUIT;
    return MenuResult::NONE;
}
