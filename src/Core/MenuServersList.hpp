#pragma once
#include <set>
#include <thread>
#include <mutex>

#include "../Network/Client.hpp"
#include "../Network/NetworkDiscovery.hpp"
#include "Config.hpp"
#include "IMenu.hpp"
class MenuServersList : public IMenu
{
    std::vector<DiscoveredServer> servers;
    std::vector<sf::Text> serverTexts;
    std::mutex serversMutex; // protège l'accès concurrent
    bool refreshRequested = false;

    // Sabler/loader
    sf::VertexArray loader;
    sf::Text loaderText;
    float startAngle = 0.f;
    sf::Text ipLabel;
    sf::Text ipField;

    sf::Text portLabel;
    sf::Text portField;

    bool ipFocused = false;
    bool portFocused = false;

    sf::Text testButton;
    sf::Text testStatus;

    bool testing = false;       // indique si un test est en cours
    std::string lastTestResult; // vide si aucun
    sf::Color lastTestColor = sf::Color::Transparent;

public:
    MenuServersList(Client &cli);

    void requestRefresh() { refreshRequested = true; };
    void update(float dt, sf::RenderWindow &w) override;
    void handleEvent(const sf::Event &e, sf::RenderWindow &w) override;
    void draw(sf::RenderWindow &w) override;
    MenuAction getAction() const override { return action; };
    void reset() override;
    void updateLoader();

private:
    Client &client;
    sf::Text title, quit;
    MenuAction action = MenuAction::NONE;
    int hoveredIndex = -1;
};
