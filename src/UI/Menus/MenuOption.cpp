#include "MenuOption.hpp"

MenuOption::MenuOption()
    : title(Config::Get().font),
      portLabel(Config::Get().font),
      tbPort(Config::Get().font),
      tbKeyUp(Config::Get().font),
      tbKeyDown(Config::Get().font),
      tbKeyLeft(Config::Get().font),
      tbKeyRight(Config::Get().font),
      tbKeyFire(Config::Get().font),
      tbKeyRocket(Config::Get().font),
      portDesc(Config::Get().font),
      saveIcon(saveTexture),
      cocheIcon(saveIcon)

{
    float cx = Config::Get().windowSize.x / 2.f;
    float cy = Config::Get().windowSize.y / 10.f;

    title.setString("Options");
    title.setCharacterSize(48);
    title.setFillColor(Config::Get().fontColor);
    title.setOrigin(title.getLocalBounds().getCenter());
    title.setPosition({cx, cy * 2});

    tbPort.setText(std::to_string(Config::Get().serverPort));
    tbPort.setLabel("Port du jeu :");
    tbPort.setLabelSpacing(300.f);
    tbPort.setMode(UITextBoxMode::LEFT);
    tbPort.setWidth(120);
    tbPort.setType(UITextBoxType::PORT);
    tbPort.setPosition({cx - tbPort.getSize().x / 2, cy * 3 - tbPort.getSize().y / 2});

    tbKeyUp.setText(Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.up)));
    tbKeyUp.setLabel("Touche UP: ");
    tbKeyUp.setLabelSpacing(300.f);
    tbKeyUp.setMode(UITextBoxMode::LEFT);
    tbKeyUp.setWidth(120);
    tbKeyUp.setType(UITextBoxType::KEY);
    tbKeyUp.setPosition({cx - tbKeyUp.getSize().x / 2, cy * 5 - tbKeyUp.getSize().y / 2});

    tbKeyDown.setText(Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.down)));
    tbKeyDown.setLabel("Touche DOWN: ");
    tbKeyDown.setLabelSpacing(300.f);
    tbKeyDown.setMode(UITextBoxMode::LEFT);
    tbKeyDown.setWidth(120);
    tbKeyDown.setType(UITextBoxType::KEY);
    tbKeyDown.setPosition({cx - tbKeyDown.getSize().x / 2, cy * 5.5f - tbKeyDown.getSize().y / 2});

    tbKeyLeft.setText(Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.left)));
    tbKeyLeft.setLabel("Touche LEFT: ");
    tbKeyLeft.setLabelSpacing(300.f);
    tbKeyLeft.setMode(UITextBoxMode::LEFT);
    tbKeyLeft.setWidth(120);
    tbKeyLeft.setType(UITextBoxType::KEY);
    tbKeyLeft.setPosition({cx - tbKeyLeft.getSize().x / 2, cy * 6 - tbKeyLeft.getSize().y / 2});

    tbKeyRight.setText(Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.right)));
    tbKeyRight.setLabel("Touche RIGHT: ");
    tbKeyRight.setLabelSpacing(300.f);
    tbKeyRight.setMode(UITextBoxMode::LEFT);
    tbKeyRight.setWidth(120);
    tbKeyRight.setType(UITextBoxType::KEY);
    tbKeyRight.setPosition({cx - tbKeyRight.getSize().x / 2, cy * 6.5f - tbKeyRight.getSize().y / 2});

    tbKeyFire.setText(Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.fire)));
    tbKeyFire.setLabel("Touche FIRE: ");
    tbKeyFire.setLabelSpacing(300.f);
    tbKeyFire.setMode(UITextBoxMode::LEFT);
    tbKeyFire.setWidth(120);
    tbKeyFire.setType(UITextBoxType::KEY);
    tbKeyFire.setPosition({cx - tbKeyFire.getSize().x / 2, cy * 7 - tbKeyFire.getSize().y / 2});

    tbKeyRocket.setText(Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.rocket)));
    tbKeyRocket.setLabel("Touche ROCKET: ");
    tbKeyRocket.setLabelSpacing(300.f);
    tbKeyRocket.setMode(UITextBoxMode::LEFT);
    tbKeyRocket.setWidth(120);
    tbKeyRocket.setType(UITextBoxType::KEY);
    tbKeyRocket.setPosition({cx - tbKeyRocket.getSize().x / 2, cy * 7.5f - tbKeyRocket.getSize().y / 2});

    portDesc.setCharacterSize(18);
    portDesc.setFillColor(Config::Get().fontColor);
    portDesc.setOrigin(portDesc.getLocalBounds().getCenter());
    portDesc.setPosition({cx, cy * 3 + tbPort.getSize().y});

    if (!saveTexture.loadFromFile("./assets/save.png"))
    {
        throw std::runtime_error("Impossible de charger save.png");
    }
    saveIcon.setTexture(saveTexture, true);
    saveIcon.setScale({.5f, .5f});
    saveIcon.setOrigin(saveIcon.getLocalBounds().getCenter());
    saveIcon.setPosition({cx + 200, cy * 9});

    if (!cocheTexture.loadFromFile("./assets/valider.png"))
    {
        throw std::runtime_error("Impossible de charger valider.png");
    }
    cocheIcon.setTexture(cocheTexture, true);
    cocheIcon.setScale({.5f, .5f});
    cocheIcon.setOrigin(cocheIcon.getLocalBounds().getCenter());
    cocheIcon.setPosition({cx + 250, cy * 9});

    quit.setTexture("./assets/bt.png");
    quit.setColor(sf::Color(255, 64, 64, 64));

    int cellWidth = quit.getTexture().getSize().x / 2;
    int cellHeight = quit.getTexture().getSize().y / 7;

    quit.setSpritesheetRects(
        {{0 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}},
        {{1 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}});

    quit.setFont(Config::Get().font);
    quit.setText("RETOUR", 40, Config::Get().fontColor);
    quit.setSize({300, 50});
    quit.setPosition({cx - quit.getSize().x / 2, cy * 9 - quit.getSize().y / 2});
    quit.onClickCallback([this]()
                         { this->action = MenuAction::GO_TO_MAIN_MENU; });

    reset();
}

void MenuOption::reset()
{
    action = MenuAction::NONE;
    tbPort.value = std::to_string(Config::Get().serverPort);

    tbKeyUp.value = Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.up));
    tbKeyUp.scancodeKey = sf::Keyboard::delocalize(Config::Get().keys.up);
    tbKeyDown.value = Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.down));
    tbKeyDown.scancodeKey = sf::Keyboard::delocalize(Config::Get().keys.down);
    tbKeyLeft.value = Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.left));
    tbKeyLeft.scancodeKey = sf::Keyboard::delocalize(Config::Get().keys.left);
    tbKeyRight.value = Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.right));
    tbKeyRight.scancodeKey = sf::Keyboard::delocalize(Config::Get().keys.right);
    tbKeyFire.value = Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.fire));
    tbKeyFire.scancodeKey = sf::Keyboard::delocalize(Config::Get().keys.fire);
    tbKeyRocket.value = Utils::keyToString(sf::Keyboard::delocalize(Config::Get().keys.rocket));
    tbKeyRocket.scancodeKey = sf::Keyboard::delocalize(Config::Get().keys.rocket);

    cursorTimer = 0.f;
    cursorVisible = true;
}
void MenuOption::update(float dt, sf::RenderWindow &w)
{
    sf::Vector2f mp = w.mapPixelToCoords(sf::Mouse::getPosition(w));

    quit.update(w);
    tbPort.update(dt);
    tbKeyUp.update(dt);
    tbKeyDown.update(dt);
    tbKeyLeft.update(dt);
    tbKeyRight.update(dt);
    tbKeyFire.update(dt);
    tbKeyRocket.update(dt);

    std::string txt;
    if (tbPort.value.length() > 0)
        txt = "Les ports " + tbPort.value + " et " + std::to_string(std::stoi(tbPort.value) + 1) + " doivent etre ouverts !";
    else
        txt = "Renseigner un port valide !";
    portDesc.setString(txt);

    portDesc.setOrigin(portDesc.getLocalBounds().getCenter());

    if (showCocheIcon)
    {
        saveCheckTimer += dt;
        if (saveCheckTimer >= 3.f)
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

        tbPort.checkFocus(mp);
        tbKeyUp.checkFocus(mp);
        tbKeyDown.checkFocus(mp);
        tbKeyLeft.checkFocus(mp);
        tbKeyRight.checkFocus(mp);
        tbKeyFire.checkFocus(mp);
        tbKeyRocket.checkFocus(mp);

        if (saveIcon.getGlobalBounds().contains(mp))
        {
            try
            {
                Config::Get().serverPort = static_cast<uint16_t>(std::stoi(tbPort.value)) != Config::Get().serverPort ? static_cast<uint16_t>(std::stoi(tbPort.value)) : Config::Get().serverPort;
                Config::Get().keys.up = sf::Keyboard::localize(tbKeyUp.scancodeKey) != Config::Get().keys.up ? sf::Keyboard::localize(tbKeyUp.scancodeKey) : Config::Get().keys.up;
                Config::Get().keys.down = sf::Keyboard::localize(tbKeyDown.scancodeKey);
                Config::Get().keys.left = sf::Keyboard::localize(tbKeyLeft.scancodeKey);
                Config::Get().keys.right = sf::Keyboard::localize(tbKeyRight.scancodeKey);
                Config::Get().keys.fire = sf::Keyboard::localize(tbKeyFire.scancodeKey);
                Config::Get().keys.rocket = sf::Keyboard::localize(tbKeyRocket.scancodeKey);

                showCocheIcon = true;
                saveCheckTimer = 0.f;
            }
            catch (...)
            {
            }
        }
    }
    tbPort.handleEvent(e);
    tbKeyUp.handleEvent(e);
    tbKeyDown.handleEvent(e);
    tbKeyLeft.handleEvent(e);
    tbKeyRight.handleEvent(e);
    tbKeyFire.handleEvent(e);
    tbKeyRocket.handleEvent(e);
}

void MenuOption::draw(sf::RenderWindow &w)
{
    w.draw(title);
    w.draw(portLabel);
    tbPort.draw(w);
    tbKeyUp.draw(w);
    tbKeyDown.draw(w);
    tbKeyLeft.draw(w);
    tbKeyRight.draw(w);
    tbKeyFire.draw(w);
    tbKeyRocket.draw(w);
    w.draw(portDesc);
    w.draw(saveIcon);
    if (showCocheIcon)
        w.draw(cocheIcon);
    quit.draw(w);
}
