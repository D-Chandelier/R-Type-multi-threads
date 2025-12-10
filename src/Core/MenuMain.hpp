#pragma once
#include "IMenu.hpp"
#include "Config.hpp"

class MenuMain : public IMenu
{
public:
    MenuMain();

    void update(float dt, sf::RenderWindow &w) override;
    void handleEvent(const sf::Event &e, sf::RenderWindow &w) override;
    void draw(sf::RenderWindow &w) override;
    MenuAction getAction() const override { return action; };
    void reset() override; // { action = MenuAction::NONE; };

private:
    sf::Text play, join, quit;
    MenuAction action = MenuAction::NONE;
};
