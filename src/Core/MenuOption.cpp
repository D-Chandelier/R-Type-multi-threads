#include "MenuOption.hpp"

MenuOption::MenuOption()
    : title(Config::Get().font),
      portLabel(Config::Get().font),
      portField(Config::Get().font),
      saveButton(Config::Get().font),
      quit(Config::Get().font)
{
    float cx = Config::Get().windowSize.x / 2.f;
    float cy = Config::Get().windowSize.y / 10.f;

    // Titre
    title.setString("Options");
    title.setCharacterSize(48);
    title.setFillColor(Config::Get().fontColor);
    title.setOrigin(title.getLocalBounds().getCenter());
    title.setPosition({cx, cy * 2});

    // Label Port
    portLabel.setString("Port de jeu :");
    portLabel.setCharacterSize(24);
    portLabel.setFillColor(Config::Get().fontColor);
    portLabel.setOrigin(portLabel.getLocalBounds().getCenter());
    portLabel.setPosition({cx - 150, cy * 4});

    // Field Port
    portField.setFont(Config::Get().font, 24);
    portField.value = std::to_string(Config::Get().serverPort);
    portField.setWidth(200);
    portField.update(0.f);
    portField.setPosition({cx + 50, cy * 4});

    // portText = std::to_string(Config::Get().serverPort);
    // portField.setString(portText);
    // portField.setCharacterSize(24);
    // portField.setFillColor(sf::Color(200, 200, 200));
    // portField.setOrigin(portField.getLocalBounds().getCenter());
    // portField.setPosition({cx + 50, cy * 4});

    // // Fond champ
    // portBackground.setSize({portField.getLocalBounds().size.x + 50, portField.getLocalBounds().size.y + 10});
    // portBackground.setFillColor(sf::Color(80, 80, 80));
    // portBackground.setOrigin(portBackground.getLocalBounds().getCenter());
    // portBackground.setPosition({portField.getPosition().x, portField.getPosition().y});

    // Bouton Sauvegarder
    saveButton.setString("Save"); // icône disque Unicode ou "Save"
    saveButton.setCharacterSize(28);
    saveButton.setFillColor(Config::Get().fontColor);
    saveButton.setOrigin(saveButton.getLocalBounds().getCenter());
    saveButton.setPosition({cx + 150 + 50, cy * 4}); // à droite du champ port

    // Bouton Retour
    quit.setString("Retour");
    quit.setCharacterSize(32);
    quit.setFillColor(Config::Get().fontColor);
    quit.setOrigin(quit.getLocalBounds().getCenter());
    quit.setPosition({cx, cy * 9});

    reset();
}

void MenuOption::reset()
{
    action = MenuAction::NONE;
    // portFocused = false;
    cursorTimer = 0.f;
    cursorVisible = true;
}
void MenuOption::update(float dt, sf::RenderWindow &w)
{
    sf::Vector2f mp = w.mapPixelToCoords(sf::Mouse::getPosition(w));

    // Gestion hover bouton Sauvegarder
    saveButton.setFillColor(saveButton.getGlobalBounds().contains(mp) ? Config::Get().hoverColor : Config::Get().fontColor);

    // Hover bouton Retour
    quit.setFillColor(quit.getGlobalBounds().contains(mp) ? Config::Get().hoverColor : Config::Get().fontColor);

    // portField.checkFocus(mp);
    portField.update(dt);

    // // Fond du champ port
    // if (portFocused)
    //     portBackground.setFillColor(sf::Color(100, 100, 150));
    // else if (portBackground.getGlobalBounds().contains(mp))
    //     portBackground.setFillColor(sf::Color(90, 90, 120));
    // else
    //     portBackground.setFillColor(sf::Color(80, 80, 80));

    // // Curseur clignotant
    // if (portFocused)
    // {
    //     cursorTimer += dt;
    //     if (cursorTimer >= 0.5f)
    //     {
    //         cursorTimer = 0.f;
    //         cursorVisible = !cursorVisible;
    //     }
    // }
}

void MenuOption::handleEvent(const sf::Event &e, sf::RenderWindow &w)
{
    if (auto *m = e.getIf<sf::Event::MouseButtonPressed>())
    {
        sf::Vector2f mp(m->position.x, m->position.y);

        // Focus champ port au clic
        portField.checkFocus(mp);
        // portFocused = portBackground.getGlobalBounds().contains(mp);

        // Boutons
        if (quit.getGlobalBounds().contains(mp))
            action = MenuAction::GO_TO_MAIN_MENU;

        if (saveButton.getGlobalBounds().contains(mp))
        {
            try
            {
                // Config::Get().serverPort = static_cast<uint16_t>(std::stoi(portField.getString().toAnsiString()));
                Config::Get().serverPort = static_cast<uint16_t>(std::stoi(portField.value));
            }
            catch (...)
            {
            }

            saveButton.setFillColor(sf::Color::Green); // feedback visuel
        }
    }
    portField.handleEvent(e);

    // // Saisie clavier
    // if (auto *t = e.getIf<sf::Event::TextEntered>())
    // {
    //     if (!portFocused)
    //         return;
    //     char c = t->unicode;

    //     if (c >= '0' && c <= '9')
    //         portText += c;
    //     else if (c == 8 && !portText.empty()) // backspace
    //         portText.pop_back();
    // }
}

void MenuOption::draw(sf::RenderWindow &w)
{
    // // Affiche texte avec curseur si focus
    // if (portFocused && cursorVisible)
    //     portField.setString(portText + "|");
    // else
    //     portField.setString(portText);

    w.draw(title);
    w.draw(portLabel);
    portField.draw(w);
    // w.draw(portBackground);
    // w.draw(portField);
    w.draw(saveButton);
    w.draw(quit);
}
