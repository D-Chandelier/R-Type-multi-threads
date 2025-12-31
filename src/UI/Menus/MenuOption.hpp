#pragma once
#include "../../Core/Config.hpp"
#include "IMenu.hpp"
#include "../Controls/UITextBox.hpp"
#include "../Controls/UIButton.hpp"
#include "../../Core/Utils.hpp"

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
    UITextBox tbPort, tbKeyUp, tbKeyDown, tbKeyLeft, tbKeyRight, tbKeyFire, tbKeyRocket;
    sf::Text portDesc;

    UIButton quit;

    sf::Texture saveTexture;
    sf::Sprite saveIcon;

    sf::Texture cocheTexture;
    sf::Sprite cocheIcon;
    bool showCocheIcon = false;
    float saveCheckTimer = 0.f;

    MenuAction action = MenuAction::NONE;

    float cursorTimer = 0.f;
    bool cursorVisible = true;
};