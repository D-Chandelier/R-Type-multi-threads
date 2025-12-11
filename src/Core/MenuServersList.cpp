#include "MenuServersList.hpp"

MenuServersList::MenuServersList(Client &cli)
    : client(cli),
      title(Config::Get().font),
      quit(Config::Get().font),
      loaderText(Config::Get().font),
      ipLabel(Config::Get().font),
      ipField(Config::Get().font),
      portLabel(Config::Get().font),
      portField(Config::Get().font),
      testButton(Config::Get().font),
      testStatus(Config::Get().font)
{
    float cx = Config::Get().windowSize.x / 2.f;
    float cy = Config::Get().windowSize.y / 10.f;

    loaderText.setString("Recherche ...");
    loaderText.setCharacterSize(24);
    loaderText.setFillColor(sf::Color::White);
    loaderText.setOrigin(loaderText.getLocalBounds().getCenter());
    loaderText.setPosition({cx, cy * 4});

    title.setString("Choisir un serveur");
    title.setOrigin(title.getLocalBounds().getCenter());
    title.setPosition({cx, cy * 3});

    quit.setString("Retour");
    quit.setOrigin(quit.getLocalBounds().getCenter());
    quit.setPosition({cx, cy * 9});

    // Champ IP
    ipLabel.setString("Adresse IP :");
    ipLabel.setCharacterSize(24);
    ipLabel.setFillColor(sf::Color::White);
    ipLabel.setOrigin(ipLabel.getLocalBounds().getCenter());
    ipLabel.setPosition({cx / 3, cy * 8});

    ipField.setString(Config::Get().serverIp);
    ipField.setCharacterSize(24);
    ipField.setFillColor(sf::Color(200, 200, 200));
    ipField.setOrigin(ipField.getLocalBounds().getCenter());
    ipField.setPosition({cx / 3 + ipLabel.getLocalBounds().size.x + 40, cy * 8});

    // Champ Port
    portLabel.setString("Port :");
    portLabel.setCharacterSize(24);
    portLabel.setFillColor(sf::Color::White);
    portLabel.setOrigin(portLabel.getLocalBounds().getCenter());
    portLabel.setPosition({cx, cy * 8});

    portField.setString(std::to_string(Config::Get().serverPort));
    portField.setCharacterSize(24);
    portField.setFillColor(sf::Color(200, 200, 200));
    portField.setOrigin(portField.getLocalBounds().getCenter());
    portField.setPosition({cx + portLabel.getLocalBounds().size.x + 40, cy * 8});

    // Bouton TEST
    testButton.setString("Joindre");
    testButton.setCharacterSize(28);
    testButton.setFillColor(Config::Get().fontColor);
    testButton.setOrigin(testButton.getLocalBounds().getCenter());
    testButton.setPosition({cx + cx / 3 * 2, cy * 8});

    // Label d’état
    testStatus.setCharacterSize(22);
    testStatus.setFillColor(sf::Color::Transparent);
    testStatus.setOrigin(testStatus.getLocalBounds().getCenter());
    testStatus.setPosition({cx, cy * 8 + 40});

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
}

void MenuServersList::update(float dt, sf::RenderWindow &w)
{
    if (ipFocused)
        ipField.setFillColor(Config::Get().hoverColor);
    else
        ipField.setFillColor(sf::Color(200, 200, 200));

    if (portFocused)
        portField.setFillColor(Config::Get().hoverColor);
    else
        portField.setFillColor(sf::Color(200, 200, 200));

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
        float cy = Config::Get().windowSize.y / 10.f;

        float startY = cy * 4;
        for (auto &s : servers)
        {
            sf::Text t(Config::Get().font);
            t.setString(s.ip + ":" + std::to_string(s.port));
            t.setOrigin({t.getLocalBounds().size.x / 2, 0});
            t.setPosition({cx, startY});
            startY += cy;
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
    hover(testButton);

    testStatus.setOrigin({testStatus.getLocalBounds().size.x / 2, testStatus.getLocalBounds().size.y / 2});

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

    testStatus.setString(lastTestResult);
    testStatus.setFillColor(lastTestColor);
}

void MenuServersList::handleEvent(const sf::Event &e, sf::RenderWindow &w)
{
    sf::Vector2f mp = w.mapPixelToCoords(sf::Mouse::getPosition(w));

    if (auto *m = e.getIf<sf::Event::MouseButtonPressed>())
    {
        ipFocused = ipField.getGlobalBounds().contains(mp);
        portFocused = portField.getGlobalBounds().contains(mp);

        if (quit.getGlobalBounds().contains(mp))
            action = MenuAction::GO_TO_MAIN_MENU;

        if (hoveredIndex >= 0)
        {
            Config::Get().serverIp = servers[hoveredIndex].ip.c_str();
            Config::Get().serverPort = servers[hoveredIndex].port;

            action = MenuAction::JOIN_SERVER;
        }

        if (testButton.getGlobalBounds().contains(mp))
        {
            if (!testing)
            {
                testing = true;
                lastTestResult = "Test en cours...";
                lastTestColor = sf::Color::Yellow;
                testStatus.setString(lastTestResult);

                std::thread([this]()
                            {
                    // Récupération des champs
                    std::string ipStr = ipField.getString().toAnsiString();
                    std::string portStr = portField.getString().toAnsiString();

                    // Résolution IP
                    auto resolved = sf::IpAddress::resolve(ipStr);
                    if (!resolved.has_value())
                    {
                        lastTestResult = "IP invalide";
                        lastTestColor = sf::Color::Red;
                        testing = false;
                        return;
                    }

                    sf::IpAddress ip = *resolved;
                    unsigned short port = 0;

                    try
                    {
                        port = static_cast<unsigned short>(std::stoi(portStr));
                    }
                    catch (...)
                    {
                        lastTestResult = "Port invalide";
                        lastTestColor = sf::Color::Red;
                        testing = false;
                        return;
                    }

                    bool ok = false;

                    // ---- Test réel ENet ----
                    ENetHost *client = enet_host_create(nullptr, 1, 1, 0, 0);
                    if (!client)
                    {
                        lastTestResult = "Impossible de créer le client ENet";
                        lastTestColor = sf::Color::Red;
                        testing = false;
                        return;
                    }

                    ENetAddress address;
                    enet_address_set_host(&address, ipStr.c_str());
                    address.port = port;

                    ENetPeer *peer = enet_host_connect(client, &address, 1, 0);
                    if (!peer)
                    {
                        lastTestResult = "Impossible de se connecter au serveur";
                        lastTestColor = sf::Color::Red;
                        enet_host_destroy(client);
                        testing = false;
                        return;
                    }

                    ENetEvent event;
                    if (enet_host_service(client, &event, 500) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
                    {
                        ok = true;
                        enet_peer_disconnect(peer, 0);
                    }

                    enet_host_destroy(client);

                    if (ok)
                    {
                        Config::Get().serverIp = ipStr.c_str();
                        Config::Get().serverPort = port;

                        action = MenuAction::JOIN_SERVER;
                    }
                    else
                    {
                        lastTestResult = "Serveur non joignable (ENet)";
                        lastTestColor = sf::Color::Red;
                    }

                    testing = false; })
                    .detach();
            }
        }
    }

    // Saisie clavier
    if (auto *t = e.getIf<sf::Event::TextEntered>())
    {
        if (ipFocused)
        {
            char c = t->unicode;
            if ((c >= '0' && c <= '9') || c == '.')
                ipField.setString(ipField.getString() + c);

            if (c == 8 && !ipField.getString().isEmpty()) // Backspace
                ipField.setString(ipField.getString().substring(0, ipField.getString().getSize() - 1));
        }

        if (portFocused)
        {
            char c = t->unicode;
            if (c >= '0' && c <= '9')
                portField.setString(portField.getString() + c);

            if (c == 8 && !portField.getString().isEmpty())
                portField.setString(portField.getString().substring(0, portField.getString().getSize() - 1));
        }
    }

    // Entrée = valider IP/Port manuels
    if (auto *k = e.getIf<sf::Event::KeyPressed>())
    {
        if (k->code == sf::Keyboard::Key::Enter)
        {
            Config::Get().serverIp = ipField.getString();
            Config::Get().serverPort = static_cast<uint16_t>(std::atoi(portField.getString().toAnsiString().c_str()));
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

    w.draw(ipLabel);
    w.draw(ipField);

    w.draw(portLabel);
    w.draw(portField);

    w.draw(testButton);
    w.draw(testStatus);

    w.draw(quit);
}
