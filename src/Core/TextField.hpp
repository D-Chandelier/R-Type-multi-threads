#pragma once
#include <SFML/Graphics.hpp>
#include <string>

struct TextField
{
    std::string value;
    sf::Text text;
    sf::RectangleShape background;

    bool focused = false;

    float cursorTimer = 0.f;
    bool cursorVisible = true;

    // Constructeur correct
    TextField(const sf::Font &font, sf::String string = "", unsigned int characterSize = 30)
        : text(font, string, characterSize) // <-- initialisation correcte
    {
        text.setFont(font);
        text.setCharacterSize(characterSize);
        text.setFillColor(Config::Get().fontColor);
        text.setString(string);
    }

    void setFont(const sf::Font &font, unsigned int size = 30)
    {
        text.setFont(font);
        text.setCharacterSize(size);
        text.setFillColor(sf::Color(200, 200, 200));
        updateBackground();
    }

    void setPosition(sf::Vector2f pos)
    {
        text.setOrigin(text.getLocalBounds().getCenter());
        text.setPosition(pos);
        updateBackground();
    }

    void setWidth(float width)
    {
        background.setSize({width, background.getLocalBounds().size.y});
    }

    void updateBackground()
    {
        sf::FloatRect bounds = text.getLocalBounds();
        background.setSize({background.getLocalBounds().size.x, bounds.size.y + 10});
        background.setOrigin(background.getLocalBounds().getCenter());
        background.setPosition(text.getPosition());
        background.setFillColor(focused ? sf::Color(100, 100, 150) : sf::Color(80, 80, 80));
    }

    void handleEvent(const sf::Event &e)
    {
        if (!focused)
            return;

        if (auto *t = e.getIf<sf::Event::TextEntered>())
        {
            char c = static_cast<char>(t->unicode);
            if ((c >= '0' && c <= '9') || c == '.')
                value += c;
            if (c == 8 && !value.empty()) // backspace
                value.pop_back();
        }
    }

    void update(float dt)
    {
        // Curseur clignotant
        if (focused)
        {
            cursorTimer += dt;
            if (cursorTimer >= 0.5f)
            {
                cursorTimer = 0.f;
                cursorVisible = !cursorVisible;
            }
        }

        text.setString(value);
    }

    void draw(sf::RenderWindow &w)
    {
        w.draw(background);

        sf::Text visual = text;

        // bounds du texte
        sf::FloatRect tb = visual.getLocalBounds();

        // calcul du centre réel du texte
        visual.setOrigin({tb.position.x + tb.size.x * 0.5f,
                          tb.position.y + tb.size.y * 0.5f});

        // background a déjà une origine centrée -> sa position = centre
        visual.setPosition(background.getPosition());

        w.draw(visual);

        // --- Curseur clignotant ---
        if (focused && cursorVisible)
        {
            // Largeur du texte actuel
            float textWidth = visual.getLocalBounds().size.x;

            // Position curseur = fin du texte centré dans le background
            sf::Vector2f cursorPos = background.getPosition();
            cursorPos.x += 5.f + textWidth * 0.5f; // fin du texte

            sf::RectangleShape cursor;
            cursor.setSize({3.f, (float)visual.getCharacterSize() * 0.6f});
            cursor.setFillColor(sf::Color::White);
            cursor.setOrigin({0.f, cursor.getSize().y * 0.5f});
            cursor.setPosition(cursorPos);

            w.draw(cursor);
        }
    }

    void checkFocus(sf::Vector2f mp)
    {
        focused = background.getGlobalBounds().contains(mp);
    }
};
