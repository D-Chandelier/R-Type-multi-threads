#include "MenuMain.hpp"

MenuMain::MenuMain()
    : play(Config::Get().font),
      join(Config::Get().font),
      option(Config::Get().font)
// quit(Config::Get().font)
{
    float cx = Config::Get().windowSize.x / 2.f;
    float cy = Config::Get().windowSize.y / 10.f;

    play.setString("Jouer (serveur)");
    play.setOrigin(play.getLocalBounds().getCenter());
    play.setPosition({cx, cy * 3});

    join.setString("Rejoindre un serveur");
    join.setOrigin(join.getLocalBounds().getCenter());
    join.setPosition({cx, cy * 4});

    option.setString("Options");
    option.setOrigin(option.getLocalBounds().getCenter());
    option.setPosition({cx, cy * 5});

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
    auto hover = [&](sf::Text &t)
    {
        if (t.getGlobalBounds().contains(mp))
            t.setFillColor(sf::Color::Yellow);
        else
            t.setFillColor(Config::Get().fontColor);
    };
    hover(play);
    hover(join);
    hover(option);
    quit.update(w);
}

void MenuMain::handleEvent(const sf::Event &e, sf::RenderWindow &w)
{
    if (auto *m = e.getIf<sf::Event::MouseButtonPressed>())
    {
        if (m->button != sf::Mouse::Button::Left)
            return;

        sf::Vector2f mp = w.mapPixelToCoords(sf::Mouse::getPosition(w));

        if (play.getGlobalBounds().contains(mp))
            action = MenuAction::START_GAME;

        if (join.getGlobalBounds().contains(mp))
            action = MenuAction::GO_TO_SERVER_LIST;

        if (option.getGlobalBounds().contains(mp))
            action = MenuAction::GO_TO_OPTION_MENU;

        // if (quit.getGlobalBounds().contains(mp))
        //     action = MenuAction::QUIT_APP;
    }
}

void MenuMain::draw(sf::RenderWindow &w)
{
    w.draw(play);
    w.draw(join);
    w.draw(option);
    quit.draw(w);
    // w.draw(quit);
}
