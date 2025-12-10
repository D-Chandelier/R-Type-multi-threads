#include "MenuMain.hpp"

MenuMain::MenuMain()
    : play(Config::Get().font),
      join(Config::Get().font),
      quit(Config::Get().font)
{
    play.setString("Jouer (serveur)");
    play.setOrigin({play.getLocalBounds().size.x / 2, play.getLocalBounds().size.y / 2});
    join.setString("Rejoindre un serveur");
    join.setOrigin({join.getLocalBounds().size.x / 2, join.getLocalBounds().size.y / 2});
    quit.setString("Quitter");
    quit.setOrigin({quit.getLocalBounds().size.x / 2, quit.getLocalBounds().size.y / 2});

    reset();
}

void MenuMain::reset()
{
    action = MenuAction::NONE;

    float cx = Config::Get().windowSize.x / 2.f;

    play.setPosition({cx, 300});
    join.setPosition({cx, 400});
    quit.setPosition({cx, 600});
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

        if (quit.getGlobalBounds().contains(mp))
            action = MenuAction::QUIT_APP;
    }
}

void MenuMain::draw(sf::RenderWindow &w)
{
    w.draw(play);
    w.draw(join);
    w.draw(quit);
}
