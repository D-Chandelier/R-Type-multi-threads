#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include "../../Core/Config.hpp"

class UIButton
{
public:
    enum class State
    {
        Normal,
        Hover,
        Pressed
    };

private:
    sf::Sprite sprite;
    sf::Texture texture;

    sf::Text text;
    const sf::Font *font = nullptr;

    sf::Vector2f position;
    sf::Vector2f size;

    sf::FloatRect bounds;

    // Rectangles dans la spritesheet selon l’état
    sf::IntRect normalRect;
    sf::IntRect hoverRect;
    sf::IntRect pressedRect;

    State state = State::Normal;

    std::function<void()> onClick; // Callback clic

public:
    UIButton() : sprite(texture),
                 text(Config::Get().font) {}

    void setTexture(const std::string &path)
    {
        if (!texture.loadFromFile(path))
            std::cout << "err texture UIButton \n";

        sprite.setTexture(texture);
        normalRect = {{0, 0}, {(int)texture.getSize().x, (int)texture.getSize().y}};
        sprite.setTextureRect(normalRect);
    }
    void setTexture(sf::Texture texture)
    {
        sprite.setTexture(texture);
        normalRect = {{0, 0}, {(int)texture.getSize().x, (int)texture.getSize().y}};
        sprite.setTextureRect(normalRect);
    }

    sf::Texture &getTexture() { return texture; }

    void setColor(sf::Color c)
    {
        sprite.setColor(c);
    }

    void setSpritesheetRects(
        sf::IntRect normal,
        sf::IntRect hover = sf::IntRect(),
        sf::IntRect pressed = sf::IntRect())
    {
        normalRect = normal;

        // Si hover non renseigné, prendre normal
        if (hover == sf::IntRect())
            hoverRect = normal;
        else
            hoverRect = hover;

        // Si pressed non renseigné, prendre hover (qui peut être normal)
        if (pressed == sf::IntRect())
            pressedRect = hoverRect;
        else
            pressedRect = pressed;

        sprite.setTextureRect(normalRect);
        updateScale();
    }

    void setFont(const sf::Font &f)
    {
        font = &f;
        text.setFont(f);
    }

    void setFontColor(sf::Color c) { text.setFillColor(c); }

    void setText(const sf::String &str, unsigned int size = 24, sf::Color color = Config::Get().fontColor)
    {
        text.setString(str);
        text.setCharacterSize(size);
        text.setFillColor(color);
    }

    void setPosition(sf::Vector2f pos)
    {
        position = pos;
        sprite.setPosition(pos);
        updateBounds();
        centerText();
    }

    void setSize(sf::Vector2f s)
    {
        size = s;
        updateScale();
        updateBounds();
        centerText();
    }
    sf::Vector2f getSize() const { return size; }

    void onClickCallback(std::function<void()> f)
    {
        onClick = std::move(f);
    }

    void update(sf::RenderWindow &window)
    {
        sf::Vector2f mp = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        bool hovered = bounds.contains(mp);

        if (hovered)
        {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            {
                state = State::Pressed;
                sprite.setTextureRect(pressedRect);
            }
            else
            {
                if (state == State::Pressed && onClick)
                    onClick();
                state = State::Hover;
                sprite.setTextureRect(hoverRect);
            }
            text.setFillColor(Config::Get().hoverColor);
        }
        else
        {
            state = State::Normal;
            sprite.setTextureRect(normalRect);
            text.setFillColor(Config::Get().fontColor);
        }
        // réappliquer le scale
        updateScale();
        centerText();
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(sprite);
        window.draw(text);
    }

    const sf::FloatRect &getBounds() const { return bounds; }

private:
    void updateBounds()
    {
        bounds = sprite.getGlobalBounds();
    }

    void updateScale()
    {
        sf::IntRect rect;
        switch (state)
        {
        case State::Normal:
            rect = normalRect;
            break;
        case State::Hover:
            rect = hoverRect;
            break;
        case State::Pressed:
            rect = pressedRect;
            break;
        }

        // facteur de scale par rapport à la portion de texture
        float scaleX = size.x / static_cast<float>(rect.size.x);
        float scaleY = size.y / static_cast<float>(rect.size.y);

        sprite.setScale({scaleX, scaleY});
    }

    void centerText()
    {
        sf::FloatRect tb = text.getLocalBounds();
        text.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
        text.setPosition(
            {position.x + size.x / 2.f,
             position.y + size.y / 2.f});
    }
};
