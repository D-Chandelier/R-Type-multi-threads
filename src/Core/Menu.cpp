#include "Menu.hpp"
#include "Utils.hpp"
#include <iostream>

Menu::Menu()
{
    ;
    if (!font.openFromFile("assets/kenvector_future_thin.ttf"))
        std::cerr << "Failed to load font!\n";
}

sf::Text Menu::makeItem(const std::string &str, float y)
{
    sf::Text t(font);
    t.setString(str);
    t.setFillColor(sf::Color::White);
    t.setCharacterSize(40);
    t.setPosition({100.f, y});
    return t;
}

int Menu::drawMainMenu(sf::RenderWindow &window)
{
    std::vector<sf::Text> items;

    items.push_back(makeItem("Play", 150));
    items.push_back(makeItem("Join game", 220));
    items.push_back(makeItem("Options", 290));
    items.push_back(makeItem("Quit", 360));

    window.clear(sf::Color(30, 30, 30));
    for (auto &t : items)
        window.draw(t);

    window.display();

    // gestion des clics
    while (auto eventOpt = window.pollEvent())
    {
        auto &event = *eventOpt; // sf::WindowEvent

        if (event.is<sf::Event::Closed>())
        {
            window.close();
        }
        if (event.is<sf::Event::MouseButtonPressed>() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
            auto mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            for (int i = 0; i < items.size(); i++)
            {
                if (items[i].getGlobalBounds().contains(mouse))
                    return i;
            }
        }
    }

    return -1;
}

int Menu::drawPauseMenu(sf::RenderWindow &window)
{
    std::vector<sf::Text> items;

    items.push_back(makeItem("Resume", 150));
    items.push_back(makeItem("Back to menu", 220));
    items.push_back(makeItem("Quit", 290));

    window.clear(sf::Color(30, 30, 30));
    for (auto &t : items)
        window.draw(t);

    window.display();

    // gestion des clics
    while (auto eventOpt = window.pollEvent())
    {
        auto &event = *eventOpt; // sf::WindowEvent

        if (event.is<sf::Event::Closed>())
        {
            window.close();
        }
        if (event.is<sf::Event::MouseButtonPressed>() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
            auto mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            for (int i = 0; i < items.size(); i++)
            {
                if (items[i].getGlobalBounds().contains(mouse))
                    return i;
            }
        }
    }

    return -1;
}

int Menu::drawOptionsMenu(sf::RenderWindow &window, Config &config)
{
    window.clear(sf::Color(30, 30, 30));

    // --- TITRE ---
    sf::Text title(font, "OPTIONS", 48);
    title.setPosition({100, 30});

    // --- CHAMPS ---
    sf::Text tName(font, "Pseudo : " + config.playerName, 40);
    tName.setPosition({200, 150});

    sf::Text tServer(font, "Server : ", 40);
    tServer.setPosition({200, 220});

    sf::Text tIp(font, "IP : " + config.serverIp, 32);
    tIp.setPosition({250, 270});

    sf::Text tPort(font, "Port : " + std::to_string(config.serverPort), 32);
    tPort.setPosition({250, 310});

    std::string localIp = Utils::getLocalIpAddress();
    sf::Text tLocal(font, "Pour joindre votre partie: \n  - IP du serveur : " + localIp + "\n  - Port : " + std::to_string(config.serverPort), 24);
    tLocal.setPosition({250, 380});

    // --- BOUTON RETOUR ---
    sf::Text back(font, "RETOUR", 40);
    back.setPosition({100, 550});

    // --- CURSEUR CLIGNOTANT ---
    bool cursorVisible = (cursorClock.getElapsedTime().asSeconds() < 0.5f);

    // Ajouter le curseur sur le champ actif
    if (cursorVisible)
    {
        if (activeField == FIELD_NAME)
            tName.setString("Pseudo : " + config.playerName + "|");
        if (activeField == FIELD_IP)
            tIp.setString("IP : " + config.serverIp + "|");
        if (activeField == FIELD_PORT)
            tPort.setString("Port : " + std::to_string(config.serverPort) + "|");
    }
    if (cursorClock.getElapsedTime().asSeconds() >= 1.f)
        cursorClock.restart();

    // --- DESSIN ---
    window.draw(title);
    window.draw(tName);
    window.draw(tServer);
    window.draw(tIp);
    window.draw(tLocal);
    window.draw(tPort);
    window.draw(back);
    window.display();

    // gestion des clics
    while (auto eventOpt = window.pollEvent())
    {
        auto &event = *eventOpt; // sf::WindowEvent

        if (event.is<sf::Event::Closed>())
        {
            window.close();
        }
        if (event.is<sf::Event::MouseButtonPressed>() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
            auto m = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            if (tName.getGlobalBounds().contains(m))
                activeField = FIELD_NAME;
            else if (tIp.getGlobalBounds().contains(m))
                activeField = FIELD_IP;
            else if (tPort.getGlobalBounds().contains(m))
                activeField = FIELD_PORT;
            else if (back.getGlobalBounds().contains(m))
                return 0;
            else
                activeField = NONE;
        }
        // --- INPUT TEXTE SUR LE CHAMP ACTIF ---
        if (activeField != NONE)
        {
            if (auto *txt = event.getIf<sf::Event::TextEntered>())
            {
                char c = static_cast<char>(txt->unicode);

                // touche effacer
                if (c == 8)
                {
                    if (activeField == FIELD_NAME && !config.playerName.empty())
                        config.playerName.pop_back();
                    else if (activeField == FIELD_IP && !config.serverIp.empty())
                        config.serverIp.pop_back();
                    else if (activeField == FIELD_PORT)
                        config.serverPort /= 10;
                }
                // caractères normaux
                else if (c >= 32 && c < 127)
                {
                    if (activeField == FIELD_NAME)
                        config.playerName += c;
                    else if (activeField == FIELD_IP)
                        config.serverIp += c;
                    else if (activeField == FIELD_PORT && isdigit(c))
                    {
                        int val = config.serverPort * 10 + (c - '0');
                        if (val <= 65535)
                            config.serverPort = val;
                    }
                }
            }
        }
    }

    return -1;
}
