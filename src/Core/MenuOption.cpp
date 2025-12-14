#include "MenuOption.hpp"

MenuOption::MenuOption()
    : title(Config::Get().font),
      portLabel(Config::Get().font),
      tbPort(Config::Get().font),
      portDesc(Config::Get().font),
      saveIcon(saveTexture),
      cocheIcon(cocheTexture)
{
    float cx = Config::Get().windowSize.x / 2.f;
    float cy = Config::Get().windowSize.y / 10.f;

    // Titre
    title.setString("Options");
    title.setCharacterSize(48);
    title.setFillColor(Config::Get().fontColor);
    title.setOrigin(title.getLocalBounds().getCenter());
    title.setPosition({cx, cy * 2});

    // TextBox Port
    tbPort.setText(std::to_string(Config::Get().serverPort));
    tbPort.setLabel("Port du jeu :");
    tbPort.setLabelSpacing(10.f);
    tbPort.setMode(UITextBoxMode::LEFT);
    tbPort.setWidth(120);
    tbPort.setType(UITextBoxType::PORT);
    tbPort.setPosition({cx - tbPort.getSize().x / 2, cy * 4 - tbPort.getSize().y / 2});

    // Description Port
    portDesc.setCharacterSize(18);
    portDesc.setFillColor(Config::Get().fontColor);
    portDesc.setOrigin(portDesc.getLocalBounds().getCenter());
    portDesc.setPosition({cx, cy * 4 + 50});

    // Bouton Sauvegarder
    if (!saveTexture.loadFromFile("./assets/save.png"))
    {
        throw std::runtime_error("Impossible de charger save.png");
    }
    saveIcon.setTexture(saveTexture, true);
    saveIcon.setScale({.5f, .5f});
    saveIcon.setOrigin(saveIcon.getLocalBounds().getCenter());
    saveIcon.setPosition({cx + 200, cy * 4});

    // Coche
    if (!cocheTexture.loadFromFile("./assets/valider.png"))
    {
        throw std::runtime_error("Impossible de charger valider.png");
    }
    cocheIcon.setTexture(cocheTexture, true);
    cocheIcon.setScale({.5f, .5f});
    cocheIcon.setOrigin(cocheIcon.getLocalBounds().getCenter());
    cocheIcon.setPosition({cx + 250, cy * 4});

    // Bouton QUIT
    quit.setTexture("./assets/bt.png");
    quit.setColor(sf::Color(255, 0, 0, 255));
    // Taille d'une cellule
    int cellWidth = quit.getTexture().getSize().x / 2;
    int cellHeight = quit.getTexture().getSize().y / 7;

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

    // Bouton Retour
    // quit.setString("Retour");
    // quit.setCharacterSize(32);
    // quit.setFillColor(Config::Get().fontColor);
    // quit.setOrigin(quit.getLocalBounds().getCenter());
    // quit.setPosition({cx, cy * 9});

    reset();
}

void MenuOption::reset()
{
    action = MenuAction::NONE;
    tbPort.value = std::to_string(Config::Get().serverPort);
    cursorTimer = 0.f;
    cursorVisible = true;
}
void MenuOption::update(float dt, sf::RenderWindow &w)
{
    sf::Vector2f mp = w.mapPixelToCoords(sf::Mouse::getPosition(w));

    // Hover bouton Retour
    // quit.setFillColor(quit.getGlobalBounds().contains(mp) ? Config::Get().hoverColor : Config::Get().fontColor);
    quit.update(w);

    // tbPort.checkFocus(mp);
    tbPort.update(dt);

    std::string txt;
    if (tbPort.value.length() > 0)
        txt = "Les ports " + tbPort.value + " et " + std::to_string(std::stoi(tbPort.value) + 1) + " doivent être ouverts !";
    else
        txt = "Renseigner un port valide !";
    portDesc.setString(txt);

    portDesc.setOrigin(portDesc.getLocalBounds().getCenter());

    // Affiche la coche verte
    if (showCocheIcon)
    {
        saveCheckTimer += dt;
        if (saveCheckTimer >= 3.f) // après 3 secondes
        {
            showCocheIcon = false;
            saveCheckTimer = 0.f;
        }
    }
}

void MenuOption::handleEvent(const sf::Event &e, sf::RenderWindow &w)
{
    if (auto *m = e.getIf<sf::Event::MouseButtonPressed>())
    {
        sf::Vector2f mp(w.mapPixelToCoords(sf::Mouse::getPosition(w)));

        // Focus champ port au clic
        tbPort.checkFocus(mp);

        // Boutons
        // if (quit.getGlobalBounds().contains(mp))
        //     action = MenuAction::GO_TO_MAIN_MENU;

        if (saveIcon.getGlobalBounds().contains(mp))
        {
            try
            {
                Config::Get().serverPort = static_cast<uint16_t>(std::stoi(tbPort.value));
                // afficher la coche
                showCocheIcon = true;
                saveCheckTimer = 0.f;
            }
            catch (...)
            {
            }
        }
    }
    tbPort.handleEvent(e);
}

void MenuOption::draw(sf::RenderWindow &w)
{
    w.draw(title);
    w.draw(portLabel);
    tbPort.draw(w);
    w.draw(portDesc);
    w.draw(saveIcon);
    if (showCocheIcon)
        w.draw(cocheIcon);
    quit.draw(w);
    // w.draw(quit);
}
