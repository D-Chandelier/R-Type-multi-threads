#pragma once
#include "IMenu.hpp"
#include "../../Core/Config.hpp"
#include "../Controls/UIButton.hpp"

class MenuInGame : public IMenu
{
public:
    MenuInGame();

    void update(float dt, sf::RenderWindow &w) override;
    void handleEvent(const sf::Event &e, sf::RenderWindow &w) override;
    void draw(sf::RenderWindow &w) override;
    void reset() override; // { action = MenuAction::NONE; };

    MenuAction getAction() const override { return action; };

private:
    sf::Text sousTitle;
    UIButton resume, quit;
    MenuAction action = MenuAction::NONE;
};
