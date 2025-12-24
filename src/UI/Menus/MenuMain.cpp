#include "MenuMain.hpp"

MenuMain::MenuMain()
{
    float cx = Config::Get().windowSize.x / 2.f;
    float cy = Config::Get().windowSize.y / 10.f;

    sf::Texture texture;
    if (!texture.loadFromFile("./assets/bt.png"))
        std::cout << "Err load ./assets/bt.png\n";
    int cellWidth = texture.getSize().x / 2;
    int cellHeight = texture.getSize().y / 7;
    sf::IntRect normal = {{0 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}};
    sf::IntRect hover = {{1 * cellWidth, 0 * cellHeight}, {cellWidth, cellHeight}};

    // Bouton PLAY
    play.setTexture("./assets/bt.png");
    play.setColor(sf::Color(128, 255, 128, 0));
    play.setSpritesheetRects(normal, hover);
    play.setFont(Config::Get().font);
    play.setText("PLAY", 40, Config::Get().fontColor);
    play.setSize({400, 50});
    play.setPosition({cx - play.getSize().x / 2, cy * 3 - play.getSize().y / 2});
    play.onClickCallback([this]()
                         { this->action = MenuAction::START_GAME; });

    // Bouton JOIN SERVER
    join.setTexture("./assets/bt.png");
    join.setColor(sf::Color(128, 255, 128, 0));
    join.setSpritesheetRects(normal, hover);
    join.setFont(Config::Get().font);
    join.setText("JOIN SERVER", 40, Config::Get().fontColor);
    join.setSize({400, 50});
    join.setPosition({cx - join.getSize().x / 2, cy * 4 - join.getSize().y / 2});
    join.onClickCallback([this]()
                         { this->action = MenuAction::GO_TO_SERVER_LIST; });

    // Bouton JOIN SERVER
    option.setTexture("./assets/bt.png");
    option.setColor(sf::Color(128, 255, 128, 0));
    option.setSpritesheetRects(normal, hover);
    option.setFont(Config::Get().font);
    option.setText("OPTIONS", 40, Config::Get().fontColor);
    option.setSize({400, 50});
    option.setPosition({cx - option.getSize().x / 2, cy * 5 - option.getSize().y / 2});
    option.onClickCallback([this]()
                           { this->action = MenuAction::GO_TO_OPTION_MENU; });

    // Bouton QUIT
    quit.setTexture("./assets/bt.png");
    quit.setColor(sf::Color(255, 64, 64, 64));
    quit.setSpritesheetRects(normal, hover);
    quit.setFont(Config::Get().font);
    quit.setText("QUITTER", 40, Config::Get().fontColor);
    quit.setSize({300, 50});
    quit.setPosition({cx - quit.getSize().x / 2, cy * 9 - quit.getSize().y / 2});
    quit.onClickCallback([this]()
                         { this->action = MenuAction::QUIT_APP; });

    reset();
}

void MenuMain::reset()
{
    action = MenuAction::NONE;
}

void MenuMain::update(float dt, sf::RenderWindow &w)
{
    // --- Hover ---
    sf::Vector2f mp = w.mapPixelToCoords(sf::Mouse::getPosition(w));
    play.update(w);
    join.update(w);
    option.update(w);
    quit.update(w);
}

void MenuMain::handleEvent(const sf::Event &e, sf::RenderWindow &w) {}

void MenuMain::draw(sf::RenderWindow &w)
{
    play.draw(w);
    join.draw(w);
    option.draw(w);
    quit.draw(w);
}
