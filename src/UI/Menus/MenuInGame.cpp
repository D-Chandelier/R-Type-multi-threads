#include "MenuInGame.hpp"

MenuInGame::MenuInGame()
    : sousTitle(Config::Get().font)
{
    float cx = Config::Get().windowSize.x / 2.f;
    float cy = Config::Get().windowSize.y / 10.f;

    sousTitle.setString("PAUSE");
    sousTitle.setFillColor(Config::Get().fontColor);
    sousTitle.setCharacterSize(40);
    sousTitle.setPosition({cx - sousTitle.getLocalBounds().getCenter().x, cy * 3 - sousTitle.getLocalBounds().getCenter().y});

    // Bouton RESUME
    resume.setTexture("./assets/bt.png");
    resume.setColor(sf::Color(128, 128, 255, 64));
    // Taille d'une cellule
    int cellWidth = resume.getTexture().getSize().x / 2;
    int cellHeight = resume.getTexture().getSize().y / 7;

    resume.setSpritesheetRects(
        {{0 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}}, // normal
        {{1 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}}  // hover
    );

    resume.setFont(Config::Get().font);
    resume.setText("REPRENDRE", 28);
    resume.setSize({300, 50});
    resume.setPosition({cx - resume.getSize().x / 2, cy * 5 - resume.getSize().y / 2});
    resume.onClickCallback([this]()
                           { this->action = MenuAction::RETURN_IN_GAME; });

    // Bouton QUIT
    quit.setTexture("./assets/bt.png");
    quit.setColor(sf::Color(255, 64, 64, 64));
    quit.setSpritesheetRects(
        {{0 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}}, // normal
        {{1 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}}  // hover
    );

    quit.setFont(Config::Get().font);
    quit.setText("MENU PRINCIPAL", 28);
    quit.setSize({350, 42});
    quit.setPosition({cx - quit.getSize().x / 2, cy * 9 - quit.getSize().y / 2});
    quit.onClickCallback([this]()
                         { this->action = MenuAction::GO_TO_MAIN_MENU; });

    reset();
}

void MenuInGame::reset()
{
    action = MenuAction::NONE;
}

void MenuInGame::update(float dt, sf::RenderWindow &w)
{
    resume.update(w);
    quit.update(w);
}

void MenuInGame::handleEvent(const sf::Event &e, sf::RenderWindow &w) {}

void MenuInGame::draw(sf::RenderWindow &w)
{
    w.draw(sousTitle);
    resume.draw(w);
    quit.draw(w);
}
