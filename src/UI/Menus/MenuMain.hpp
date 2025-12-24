#pragma once
#include "IMenu.hpp"
#include "../../Core/Config.hpp"
#include "../Controls/UIButton.hpp"
#include <SFML/Graphics.hpp>

class MenuMain : public IMenu
{
public:
    MenuMain();

    void update(float dt, sf::RenderWindow &w) override;
    void handleEvent(const sf::Event &e, sf::RenderWindow &w) override;
    void draw(sf::RenderWindow &w) override;
    void reset() override; // { action = MenuAction::NONE; };

    MenuAction getAction() const override { return action; };

private:
    // sf::Text play, join, option;
    UIButton play, join, option, quit;
    MenuAction action = MenuAction::NONE;
};
