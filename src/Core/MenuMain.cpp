#include "MenuMain.hpp"

MenuMain::MenuMain()
    : play(Config::Get().font),
      join(Config::Get().font),
      option(Config::Get().font),
      quit(Config::Get().font)
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

    quit.setString("Quitter");
    quit.setOrigin(quit.getLocalBounds().getCenter());
    quit.setPosition({cx, cy * 9});

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
    hover(quit);
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

        if (quit.getGlobalBounds().contains(mp))
            action = MenuAction::QUIT_APP;
    }
}

void MenuMain::draw(sf::RenderWindow &w)
{
    w.draw(play);
    w.draw(join);
    w.draw(option);
    w.draw(quit);
}
