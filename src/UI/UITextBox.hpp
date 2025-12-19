#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <string>
#include <sstream>

#include "../Core/Config.hpp"
#include "../Core/Utils.hpp"

#undef min
#undef max

enum class UITextBoxMode
{
    ABOVE, // label au-dessus
    LEFT   // label à gauche
};
enum class UITextBoxType
{
    PORT,
    IP,
    TEXT,
    KEY
};
struct UITextBox
{
    // Public
    std::string value = "";
    UITextBoxType type = UITextBoxType::TEXT;

    sf::Text label;
    sf::Text text;

    sf::RectangleShape box;     // zone input
    sf::RectangleShape outline; // contour

    UITextBoxMode mode = UITextBoxMode::LEFT;

    bool focused = false;

    sf::Vector2f position;
    sf::Vector2f origin;
    float width = 200.f;
    float height = 40.f;
    float labelSpacing = 0.f;
    sf::FloatRect bounds;
    sf::Vector2f totalSize;

    // Curseur
    bool cursorVisible = true;
    float cursorTimer = 0.f;

    // KeyCode
    sf::Keyboard::Scancode scancodeKey = sf::Keyboard::Scancode::Unknown; // touche enregistrée

    UITextBox(const sf::Font &font) : type(UITextBoxType::TEXT),
                                      width(200),
                                      height(26),
                                      label(Config::Get().font),
                                      text(Config::Get().font)
    {
        label.setCharacterSize(22);
        label.setFillColor(sf::Color(220, 220, 220));
        label.setString("");

        text.setCharacterSize(22);
        text.setFillColor(sf::Color::White);

        box.setSize({width, height});
        box.setFillColor(sf::Color(60, 60, 60));
        box.setOutlineThickness(0);

        outline.setSize({width, height});
        outline.setFillColor(sf::Color::Transparent);
        outline.setOutlineThickness(2.f);
        outline.setOutlineColor(sf::Color(90, 90, 90));
    }

    void setLabel(const std::string &s)
    {
        label.setString(s);
        updateLayout();
    }

    void setText(const std::string &s)
    {
        text.setString(s);
        updateLayout();
    }

    void setSize(sf::Vector2f s)
    {
        height = s.y;
        width = s.x;
        box.setSize({width, height});
        updateLayout();
    }
    
    void setWidth(float w)
    {
        width = w;
        box.setSize({width, height});
        updateLayout();
    }

    void setHeight(float h)
    {
        height = h;
        box.setSize({width, height});
        updateLayout();
    }

    void setPosition(const sf::Vector2f &p)
    {
        position = p;
        updateLayout();
    }

    void setLabelSpacing(const float s)
    {
        labelSpacing = s;
        updateLayout();
    }

    void setMode(UITextBoxMode m)
    {
        mode = m;
        updateLayout();
    }

    void setType(UITextBoxType t)
    {
        type = t;
        updateLayout();
    }

    sf::FloatRect getGlobalBounds() const { return bounds; }
    sf::Vector2f getSize() const { return totalSize; }
    sf::Vector2f getPosition() const { return position; }

    void draw(sf::RenderWindow &w)
    {
        updateLayout();

        w.draw(box);
        w.draw(label);
        w.draw(text);

        if (focused && cursorVisible)
        {
            sf::FloatRect tb = text.getLocalBounds();
            sf::Vector2f textPos = text.getPosition(); // position centrale du texte

            // Position du curseur : fin du texte
            sf::Vector2f cursorPos;
            cursorPos.x = textPos.x - tb.size.x * 0.5f + tb.size.x + 5.f; // gauche du texte + largeur
            cursorPos.y = textPos.y;                                      // centré verticalement

            sf::RectangleShape cursor({2.f, text.getCharacterSize() * 0.75f});
            cursor.setOrigin({1.f, cursor.getSize().y * 0.5f});
            cursor.setPosition(cursorPos);
            cursor.setFillColor(sf::Color::White);
            w.draw(cursor);
        }
    }

    void update(float dt)
    {
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
        updateLayout();
    }

    void checkFocus(sf::Vector2f mp)
    {
        focused = bounds.contains(mp);
        updateLayout();
    }

    void handleEvent(const sf::Event &e)
    {
        if (!focused)
            return;

            // --- KEY BINDING ---
        if (type == UITextBoxType::KEY)
        {
            if (auto *k = e.getIf<sf::Event::KeyPressed>())
            {
                handleKey(k->scancode);
            }
            return; // IMPORTANT : on ne traite rien d'autre
        }

        if (auto *t = e.getIf<sf::Event::TextEntered>())
        {
            char c = static_cast<char>(t->unicode);

            switch (type)
            {
            case UITextBoxType::PORT:
                handlePort(c);
                break;

            case UITextBoxType::IP:
                handleIP(c);
                break;

            case UITextBoxType::TEXT:
                handleFreeText(c);
                break;
            }
        }
    }

private:
    void updateLayout()
    {
        // Mesures des textes
        sf::FloatRect lb = label.getLocalBounds();
        sf::FloatRect tb = text.getLocalBounds();

        float labelWidth = lb.size.x;
        float labelHeight = lb.size.y;
        float textHeight = tb.size.y;

        text.setString(value);

        // sf::Vector2f totalSize;

        if (mode == UITextBoxMode::ABOVE)
        {
            totalSize.x = std::max(labelWidth, width);
            totalSize.y = labelHeight + labelSpacing + height;

            // Position label centré
            label.setOrigin({lb.position.x + lb.size.x * 0.5f, lb.position.y + lb.size.y * 0.5f});
            label.setPosition({position.x - origin.x + totalSize.x * 0.5f,
                               position.y - origin.y + labelHeight * 0.5f});

            // Position box
            box.setOrigin({width * 0.5f, height * 0.5f});
            box.setPosition({position.x - origin.x + totalSize.x * 0.5f,
                             position.y - origin.y + labelHeight + labelSpacing + height * 0.5f});
        }
        else
        { // Label à gauche
            // totalSize.x = labelWidth + labelSpacing + width;
            totalSize.x = labelSpacing + width;
            totalSize.y = std::max(labelHeight, height);

            // Label
            //label.setOrigin({lb.position.x, lb.position.y + lb.size.y * 0.5f});
            label.setOrigin({lb.position.x, lb.position.y + lb.size.y * 0.5f});
            // label.setPosition({position.x - origin.x - labelSpacing,
            //                    position.y - origin.y + totalSize.y * 0.5f});

            label.setPosition({position.x - labelSpacing / 2.f + width,
                               position.y + totalSize.y * 0.5f});

            // Box
            box.setOrigin({width * 0.5f, height * 0.5f});
            box.setPosition({position.x - origin.x + labelSpacing + width * 0.5f,
                             position.y - origin.y + totalSize.y * 0.5f});
        }

        // Texte centré dans la box
        text.setOrigin({tb.position.x + tb.size.x * 0.5f, tb.position.y + tb.size.y * 0.5f});
        text.setPosition(box.getPosition());

        // Bounds global du widget
        bounds = {{position.x - origin.x,
                   position.y - origin.y},
                  {totalSize.x,
                   totalSize.y}};
    }

    void handlePort(char c)
    {
        if (c >= '0' && c <= '9')
        {
            if (value.size() < 5)
            {
                std::string nv = value + c;

                try
                {
                    int p = std::stoi(nv);
                    if (p > 65534)
                        nv = "65534";

                    value = nv;
                }
                catch (...)
                {
                }
            }
        }
        else if (c == 8 && !value.empty())
            value.pop_back();
        else if (c == 127)
            value.clear();
        updateLayout();
    }

    void handleIP(char c)
    {
        if ((c >= '0' && c <= '9') || c == '.')
        {
            std::string nv = value + c;
            std::istringstream ss(nv);
            std::string octet;
            int count = 0;
            bool valid = true;

            while (std::getline(ss, octet, '.'))
            {
                count++;
                if (count > 4 || octet.size() > 3)
                {
                    valid = false;
                    break;
                }

                if (!octet.empty())
                {
                    try
                    {
                        int v = std::stoi(octet);
                        if (v < 0 || v > 255)
                            valid = false;
                    }
                    catch (...)
                    {
                        valid = false;
                    }
                }
            }

            if (valid && std::count(nv.begin(), nv.end(), '.') <= 3)
                value = nv;
        }
        else if (c == 8 && !value.empty())
            value.pop_back();
    }

    void handleKey(sf::Keyboard::Scancode sc)
    {
        // Convertir la Key en Scancode puis en string
        //sf::Keyboard::Scancode sc = sf::Keyboard::keyToScancode(k);
        value = keyToString(sc);
        scancodeKey = sc;
        updateLayout();
        focused = false;    
        
    }

    void handleFreeText(char c)
    {
        if ((c >= 32 && c < 127))
            value += c;
        else if (c == 8 && !value.empty())
            value.pop_back();
        else if (c == 127)
            value.clear();
    }
};