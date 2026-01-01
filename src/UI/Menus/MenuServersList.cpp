#include "MenuServersList.hpp"

MenuServersList::MenuServersList(Client &cli, NetworkDiscovery &nd)
    : client(cli),
      nd(nd),
      title(Config::Get().font),
      loaderText(Config::Get().font),
      tbIp(Config::Get().font),
      tbPort(Config::Get().font),
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

    // TextBox IP
    tbIp.setText(Config::Get().serverIp);
    tbIp.setLabel("Adresse IP :");
    tbIp.setLabelSpacing(300.f);
    tbPort.setMode(UITextBoxMode::LEFT);
    tbIp.setWidth(220);
    tbIp.setType(UITextBoxType::IP);
    tbIp.setPosition({cx - 20 - tbIp.getSize().x / 2.f, cy * 6.5f});

    // TextBox Port
    tbPort.setText(std::to_string(Config::Get().serverPort));
    tbPort.setLabel("Port :");
    tbPort.setLabelSpacing(300.f);
    tbPort.setMode(UITextBoxMode::LEFT);
    tbPort.setWidth(120);
    tbPort.setType(UITextBoxType::PORT);
    tbPort.setPosition({cx + 30.f - tbPort.getSize().x / 2.f, cy * 7});

    // Bouton TEST
    buttonTest.setTexture("./assets/bt.png");
    buttonTest.setColor(sf::Color(128, 255, 128, 255));
    // Taille d'une cellule
    int cellWidth = buttonTest.getTexture().getSize().x / 2;
    int cellHeight = buttonTest.getTexture().getSize().y / 7;

    buttonTest.setSpritesheetRects(
        {{0 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}}, // normal
        {{1 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}}  // hover
    );

    buttonTest.setFont(Config::Get().font);
    buttonTest.setText("Try", 24, sf::Color::Black);
    buttonTest.setSize({200, 28});
    buttonTest.setPosition({cx - buttonTest.getSize().x / 2.f, cy * 8});
    buttonTest.onClickCallback([this]()
                               { this->buttonTest_Click(); });

    // Label d’état
    testStatus.setCharacterSize(22);
    testStatus.setFillColor(sf::Color::Transparent);
    testStatus.setOrigin(testStatus.getLocalBounds().getCenter());
    testStatus.setPosition({cx, cy * 8 + -40});

    // Bouton QUIT
    quit.setTexture("./assets/bt.png");
    quit.setColor(sf::Color(255, 64, 64, 64));

    quit.setSpritesheetRects(
        {{0 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}}, // normal
        {{1 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}}  // hover
    );

    quit.setFont(Config::Get().font);
    quit.setText("RETOUR", 40, Config::Get().fontColor);
    quit.setSize({300, 50});
    quit.setPosition({cx - quit.getSize().x / 2, cy * 9 - quit.getSize().y / 2});
    quit.onClickCallback([this]()
                         { this->action = MenuAction::GO_TO_MAIN_MENU; });

    reset();
}

void MenuServersList::buttonTest_Click()
{
    if (!testing)
    {
        testing = true;
        lastTestResult = "Test en cours...";
        lastTestColor = sf::Color::Yellow;
        testStatus.setString(lastTestResult);

        std::thread([this]()
                    {
            
            // Résolution IP
            auto resolved = sf::IpAddress::resolve(tbIp.value);
            if (!resolved.has_value())
            {
                std::lock_guard<std::mutex> lock(testMutex);
                lastTestResult = "IP invalide";
                lastTestColor = sf::Color::Red;
                testing = false;
                return;
            }

            sf::IpAddress ip = *resolved;
            unsigned short port = 0;

            try
            {
                port = static_cast<unsigned short>(std::stoi(tbPort.value));
            }
            catch (...)
            {
                std::lock_guard<std::mutex> lock(testMutex);
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
                std::lock_guard<std::mutex> lock(testMutex);
                lastTestResult = "Impossible de créer le client ENet";
                lastTestColor = sf::Color::Red;
                testing = false;
                return;
            }

            ENetAddress address;
            enet_address_set_host(&address, tbIp.value.c_str());
            address.port = port;

            ENetPeer *peer = enet_host_connect(client, &address, 1, 0);
            if (!peer)
            {
                std::lock_guard<std::mutex> lock(testMutex);
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
                Config::Get().serverIp = tbIp.value.c_str();
                Config::Get().serverPort = port;

                action = MenuAction::JOIN_SERVER;
            }
            else
            {
                std::lock_guard<std::mutex> lock(testMutex);
                lastTestResult = "Serveur non joignable (ENet)";
                lastTestColor = sf::Color::Red;
            }                    
            testing = false; })
            .detach();
    }
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
    {
        std::lock_guard<std::mutex> lock(serversMutex);
        servers.clear();
        serverTexts.clear();
    }
    tbIp.value = Config::Get().serverIp;
    tbPort.value = std::to_string(Config::Get().serverPort);
    lastTestResult = "";
}

void MenuServersList::update(float dt, sf::RenderWindow &w)
{
    sf::Vector2f mp = w.mapPixelToCoords(sf::Mouse::getPosition(w));

    tbPort.update(dt);
    tbIp.update(dt);
    buttonTest.update(w);

    // Rotation
    startAngle += 180.f * dt;
    if (startAngle >= 360.f)
        startAngle -= 360.f;

    // Recrée le loader
    updateLoader();

    // Scanner les serveurs toutes les secondes
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
                auto newServers = nd.getDiscoveredServers();

                std::set<std::pair<std::string,uint16_t>> uniqueSet;
                std::vector<DiscoveredServer> uniqueServers;
                for (auto &s : newServers)
                    if (uniqueSet.insert({s.ip, s.port}).second)
                        uniqueServers.push_back(s);

                // Accès sécurisé
                {
                    std::lock_guard<std::mutex> lock(serversMutex);
                    servers = uniqueServers;

                    // Recrée serverTexts dans le thread principal sécurisé
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
                        startY += cy * .66f;
                        serverTexts.push_back(t);
                    }
                }
                refreshRequested = false; })
                .detach(); // détaché pour ne pas bloquer
        }
    }

    // Hover et mise à jour couleur serveur
    hoveredIndex = -1;
    {
        std::lock_guard<std::mutex> lock(serversMutex);
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

    auto hover = [&](sf::Text &t)
    {
        if (t.getGlobalBounds().contains(mp))
            t.setFillColor(Config::Get().hoverColor);
        else
            t.setFillColor(Config::Get().fontColor);
    };

    quit.update(w);

    testStatus.setString(lastTestResult);
    testStatus.setOrigin(testStatus.getLocalBounds().getCenter());
    testStatus.setFillColor(lastTestColor);
}

void MenuServersList::handleEvent(const sf::Event &e, sf::RenderWindow &w)
{
    sf::Vector2f mp = w.mapPixelToCoords(sf::Mouse::getPosition(w));

    if (auto *m = e.getIf<sf::Event::MouseButtonPressed>())
    {
        // Focus champ port et ip au clic
        tbPort.checkFocus(mp);
        tbIp.checkFocus(mp);

        // Accès sécurisé pour servers
        if (hoveredIndex >= 0)
        {
            std::lock_guard<std::mutex> lock(serversMutex);
            if (hoveredIndex < servers.size())
            {
                Config::Get().serverIp = servers[hoveredIndex].ip;
                Config::Get().serverPort = servers[hoveredIndex].port;
                action = MenuAction::JOIN_SERVER;
            }
        }
    }
    tbPort.handleEvent(e);
    tbIp.handleEvent(e);
}

void MenuServersList::draw(sf::RenderWindow &w)
{
    w.draw(title);
    {
        std::lock_guard<std::mutex> lock(serversMutex);
        if (serverTexts.empty())
        {
            w.draw(loader);
            w.draw(loaderText);
        }
        else
        {
            for (auto &t : serverTexts)
                w.draw(t);
        }
    }

    tbIp.draw(w);
    tbPort.draw(w);
    buttonTest.draw(w);
    w.draw(testStatus);
    quit.draw(w);
}
