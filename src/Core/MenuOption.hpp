#pragma once
#include "Config.hpp"
#include "IMenu.hpp"
#include "TextField.hpp"

class MenuOption : public IMenu
{

public:
    MenuOption();

    void update(float dt, sf::RenderWindow &w) override;
    void handleEvent(const sf::Event &e, sf::RenderWindow &w) override;
    void draw(sf::RenderWindow &w) override;
    void reset() override; // { action = MenuAction::NONE; };

    MenuAction getAction() const override { return action; };

private:
    sf::Text title;

    sf::Text portLabel;
    TextField portField;
    // sf::Text portField;
    // sf::RectangleShape portBackground;
    // bool portFocused = false;
    // std::string portText;

    sf::Text saveButton;
    sf::Text quit;

    MenuAction action = MenuAction::NONE;

    float cursorTimer = 0.f;
    bool cursorVisible = true;
};