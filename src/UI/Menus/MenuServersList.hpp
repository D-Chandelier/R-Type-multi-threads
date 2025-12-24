#pragma once
#include <set>
#include <thread>
#include <mutex>

#include "../../Network/Client/Client.hpp"
#include "../../Network/Protocols/NetworkDiscovery.hpp"
#include "../Controls/UITextBox.hpp"
#include "../Controls/UIButton.hpp"
#include "../../Core/Config.hpp"
#include "IMenu.hpp"

class MenuServersList : public IMenu
{
    std::vector<DiscoveredServer> servers;
    std::vector<sf::Text> serverTexts;
    std::mutex serversMutex; // protège l'accès concurrent
    std::mutex testMutex;    // protège l'accès concurrent
    bool refreshRequested = false;
    Client &client;
    NetworkDiscovery &nd;

    // Sabler/loader
    sf::VertexArray loader;
    sf::Text loaderText;
    float startAngle = 0.f;

    UITextBox tbIp;
    UITextBox tbPort;

    bool ipFocused = false;
    bool portFocused = false;

    UIButton buttonTest;
    sf::Text testStatus;

    bool testing = false;       // indique si un test est en cours
    std::string lastTestResult; // vide si aucun
    sf::Color lastTestColor = sf::Color::Transparent;

public:
    MenuServersList(Client &cli, NetworkDiscovery &nd);

    void requestRefresh() { refreshRequested = true; };
    void update(float dt, sf::RenderWindow &w) override;
    void handleEvent(const sf::Event &e, sf::RenderWindow &w) override;
    void draw(sf::RenderWindow &w) override;
    MenuAction getAction() const override { return action; };
    void reset() override;
    void updateLoader();
    void buttonTest_Click();

private:
    sf::Text title;
    UIButton quit;
    MenuAction action = MenuAction::NONE;
    int hoveredIndex = -1;
};
