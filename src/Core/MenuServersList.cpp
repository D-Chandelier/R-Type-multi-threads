#include "MenuServersList.hpp"

std::vector<MenuServerStar> serverStars;

MenuServersList::MenuServersList()
    : title(Config::Get().font),
      ssTitle(Config::Get().font),
      quit(Config::Get().font)
{
    title.setString(Config::Get().title);
    title.setCharacterSize(80);
    title.setFillColor(Config::Get().fontColor);
    title.setOrigin({title.getGlobalBounds().size.x / 2, title.getGlobalBounds().size.y / 2});
    title.setPosition({static_cast<float>(Config::Get().windowSize.x / 2), static_cast<float>(Config::Get().windowSize.y / 8)});

    ssTitle.setString("Sélectionner le serveur à joindre:");
    ssTitle.setCharacterSize(40);
    ssTitle.setFillColor(Config::Get().fontColor);
    ssTitle.setOrigin({ssTitle.getGlobalBounds().size.x / 2, ssTitle.getGlobalBounds().size.y / 2});
    ssTitle.setPosition({static_cast<float>(Config::Get().windowSize.x / 2), static_cast<float>(Config::Get().windowSize.y / 8 * 2)});

    quit.setString("Retour");
    quit.setCharacterSize(60);
    quit.setFillColor(Config::Get().fontColor);
    quit.setOrigin({quit.getGlobalBounds().size.x / 2, quit.getGlobalBounds().size.y / 2});
    quit.setPosition({static_cast<float>(Config::Get().windowSize.x / 2), static_cast<float>(Config::Get().windowSize.y / 8 * 6)});

    // --- Génération des étoiles ---
    serverStars.reserve(50);
    for (int i = 0; i < 50; ++i)
    {
        MenuServerStar s;
        s.pos = {static_cast<float>(rand() % Config::Get().windowSize.x),
                 static_cast<float>(rand() % Config::Get().windowSize.y)};
        s.speed = 30.f + static_cast<float>(rand() % 70);
        s.size = 1.f + static_cast<float>(rand() % 3);
        s.color = sf::Color(150 + rand() % 105, 150 + rand() % 105, 150 + rand() % 105);
        serverStars.push_back(s);
    }
}

MenuResult MenuServersList::showServerList(sf::RenderWindow &window, Client &client, uint16_t discoveryPort)
{
    sf::Clock clock;
    float timeAccumulator = 0.f;

    NetworkDiscovery nd;
    auto servers = nd.scanLAN(discoveryPort, 2000);

    // --- SUPPRIMER LES DOUBLONS ---
    std::set<std::pair<std::string, uint16_t>> uniqueSet;
    std::vector<DiscoveredServer> uniqueServers;

    for (auto &s : servers)
    {
        if (uniqueSet.insert({s.ip, s.port}).second)
            uniqueServers.push_back(s);
    }
    servers = uniqueServers;

    if (servers.empty())
    {
        std::cout << "[JOIN] Aucun serveur trouvé...\n";
        return MenuResult::NONE;
    }

    sf::Font font = Config::Get().font;
    const sf::Color normalColor = Config::Get().fontColor;
    const sf::Color hoverColor = Config::Get().overColor; // Jaune

    std::vector<sf::Text> serverTexts;

    const float startY = Config::Get().windowSize.y / 8 * 3;
    const float spacing = 60.f;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        sf::Text text(font);
        text.setString(servers[i].ip + ":" + std::to_string(servers[i].port));
        text.setCharacterSize(40);
        text.setFillColor(normalColor);
        text.setOrigin({text.getGlobalBounds().size.x / 2, text.getGlobalBounds().size.y / 2});
        text.setPosition({static_cast<float>(Config::Get().windowSize.x / 2), startY + i * spacing});
        serverTexts.push_back(text);

        std::cout << "Serveur trouvé : " << servers[i].ip << ":" << servers[i].port << "\n";
    }

    bool done = false;

    while (window.isOpen() && !done)
    {
        float dt = clock.restart().asSeconds();
        timeAccumulator += dt;

        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos((float)mouse.x, (float)mouse.y);

        // --- Reset couleur texte ---
        for (auto &t : serverTexts)
            t.setFillColor(normalColor);

        // --- Hover detection ---
        int hoveredIndex = -1;
        for (size_t i = 0; i < serverTexts.size(); ++i)
        {
            if (serverTexts[i].getGlobalBounds().contains(mousePos))
            {
                serverTexts[i].setFillColor(hoverColor);
                hoveredIndex = static_cast<int>(i);
            }
        }

        while (auto eventOpt = window.pollEvent())
        {
            sf::Vector2f mp(sf::Mouse::getPosition(window));

            auto &event = *eventOpt;

            if (auto *e = event.getIf<sf::Event::Closed>())
            {
                window.close();
                return MenuResult::NONE;
            }

            // Clic sur un texte
            if (auto *e = event.getIf<sf::Event::MouseButtonPressed>())
            {
                if (e->button == sf::Mouse::Button::Left && hoveredIndex >= 0)
                {
                    client.connectTo(servers[hoveredIndex].ip.c_str(), servers[hoveredIndex].port);
                    std::cout << "[JOIN] Connected to " << servers[hoveredIndex].ip
                              << ":" << servers[hoveredIndex].port << "\n";
                    done = true;
                    return MenuResult::JOIN;
                }
                if (e->button == sf::Mouse::Button::Left && quit.getGlobalBounds().contains(mp))
                {
                    return MenuResult::NONE; // pour revenir au menu principal;
                }
            }
        }

        // --- Hover ---
        sf::Vector2f mp(sf::Mouse::getPosition(window));
        auto hover = [&](sf::Text &t)
        {
            if (t.getGlobalBounds().contains(mp))
                t.setFillColor(sf::Color::Yellow);
            else
                t.setFillColor(Config::Get().fontColor);
        };
        hover(quit);

        // --- Mise à jour étoiles ---
        for (auto &s : serverStars)
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
        for (auto &s : serverStars)
        {
            pixel.setSize({s.size, s.size});
            pixel.setPosition(s.pos);
            pixel.setFillColor(s.color);
            window.draw(pixel);
        }
        window.draw(title);
        window.draw(ssTitle);
        for (auto &t : serverTexts)
            window.draw(t);
        window.draw(quit);
        window.display();
    }

    return MenuResult::NONE;
}
