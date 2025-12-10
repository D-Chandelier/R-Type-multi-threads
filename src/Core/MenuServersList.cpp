#include "MenuServersList.hpp"

MenuServersList::MenuServersList(Client &cli)
    : client(cli),
      title(Config::Get().font),
      quit(Config::Get().font),
      loaderText(Config::Get().font)
{
    loaderText.setString("Recherche ...");
    loaderText.setCharacterSize(24);
    loaderText.setFillColor(sf::Color::White);
    loaderText.setOrigin({loaderText.getLocalBounds().size.x / 2.f, 0});
    loaderText.setPosition({Config::Get().windowSize.x / 2.f, 450.f});

    title.setString("Choisir un serveur");
    title.setOrigin({title.getLocalBounds().size.x / 2, title.getLocalBounds().size.y / 2});

    quit.setString("Retour");
    quit.setOrigin({quit.getLocalBounds().size.x / 2, quit.getLocalBounds().size.y / 2});

    reset();
}

void MenuServersList::updateLoader()
{
    loader.clear();
    sf::Vector2f center(Config::Get().windowSize.x / 2.f, 400.f);
    float radius = 25.f;
    int segments = 45;
    float arcSize = 135.f;
    float thickness = 4.f; // épaisseur de l’arc

    loader.setPrimitiveType(sf::PrimitiveType::TriangleStrip);

    for (int i = 0; i <= segments; i++)
    {
        float angle = (startAngle + i * arcSize / segments) * 3.14159f / 180.f;
        sf::Vector2f dir(std::cos(angle), std::sin(angle));

        sf::Vector2f outer = center + dir * (radius + thickness / 2.f);
        sf::Vector2f inner = center + dir * (radius - thickness / 2.f);

        // Alpha progressif pour effet de flux
        float alpha = 50.f + 205.f * (float(i) / segments);

        loader.append(sf::Vertex(inner, sf::Color(255, 255, 0, static_cast<std::uint8_t>(alpha))));
        loader.append(sf::Vertex(outer, sf::Color(255, 255, 0, static_cast<std::uint8_t>(alpha))));
    }
}

void MenuServersList::reset()
{
    action = MenuAction::NONE;
    servers.clear();
    serverTexts.clear();

    float cx = Config::Get().windowSize.x / 2.f;

    title.setPosition({cx, 300});
    quit.setPosition({cx, 600});
}

void MenuServersList::update(float dt, sf::RenderWindow &w)
{
    // Rotation
    startAngle += 180.f * dt;
    if (startAngle >= 360.f)
        startAngle -= 360.f;

    // Recrée le loader
    updateLoader();

    static float accumulator = 0.f;
    accumulator += dt;
    if (accumulator > 1.f)
    {
        accumulator = 0.f;
        if (!refreshRequested)
        {
            refreshRequested = true;
            // Lance le scan dans un thread séparé
            std::thread([this]()
                        {
                NetworkDiscovery nd;
                auto newServers = nd.scanLAN(Config::Get().discoveryPort, 1000);

                std::set<std::pair<std::string,uint16_t>> uniqueSet;
                std::vector<DiscoveredServer> uniqueServers;
                for (auto &s : newServers)
                    if (uniqueSet.insert({s.ip, s.port}).second)
                        uniqueServers.push_back(s);

                // Verrouillage avant modification des vecteurs partagés
                std::lock_guard<std::mutex> lock(serversMutex);
                servers = uniqueServers;

                refreshRequested = false; })
                .detach(); // détaché pour ne pas bloquer
        }
    }

    // Recrée les sf::Text dans le thread principal pour SFML
    {
        std::lock_guard<std::mutex> lock(serversMutex);
        serverTexts.clear();
        float cx = Config::Get().windowSize.x / 2.f;
        float startY = 350;
        for (auto &s : servers)
        {
            sf::Text t(Config::Get().font);
            t.setString(s.ip + ":" + std::to_string(s.port));
            t.setOrigin({t.getLocalBounds().size.x / 2, 0});
            t.setPosition({cx, startY});
            startY += 60;
            serverTexts.push_back(t);
        }
    }

    // --- Hover ---
    sf::Vector2f mp = w.mapPixelToCoords(sf::Mouse::getPosition(w));
    auto hover = [&](sf::Text &t)
    {
        if (t.getGlobalBounds().contains(mp))
            t.setFillColor(Config::Get().hoverColor);
        else
            t.setFillColor(Config::Get().fontColor);
    };
    hover(quit);

    // Hover sur les serveurs
    hoveredIndex = -1;
    for (int i = 0; i < serverTexts.size(); i++)
    {
        if (serverTexts[i].getGlobalBounds().contains(mp))
        {
            serverTexts[i].setFillColor(Config::Get().hoverColor);
            hoveredIndex = i;
        }
        else
        {
            serverTexts[i].setFillColor(Config::Get().fontColor);
        }
    }
}

void MenuServersList::handleEvent(const sf::Event &e, sf::RenderWindow &w)
{
    if (auto *m = e.getIf<sf::Event::MouseButtonPressed>())
    {
        sf::Vector2f mp = w.mapPixelToCoords(sf::Mouse::getPosition(w));

        if (quit.getGlobalBounds().contains(mp))
            action = MenuAction::GO_TO_MAIN_MENU;

        if (hoveredIndex >= 0)
        {
            Config::Get().serverIp = servers[hoveredIndex].ip.c_str();
            Config::Get().serverPort = servers[hoveredIndex].port;

            action = MenuAction::JOIN_SERVER;
        }
    }
}

void MenuServersList::draw(sf::RenderWindow &w)
{
    w.draw(title);
    if (serverTexts.empty())
    {
        // Affiche loader
        w.draw(loader);
        w.draw(loaderText);
    }
    else
    {
        for (auto &t : serverTexts)
            w.draw(t);
    }
    w.draw(quit);
}
