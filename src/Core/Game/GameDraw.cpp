#include "Game.hpp"

void Game::draw(float dt)
{
    window.clear(Config::Get().backgroundColor);

    if (state != GameState::IN_GAME)
    {
        menuBackground.draw(window);
        currentMenu->draw(window);
    }
    else
        drawGameplay();

    window.display();
}

void Game::drawGameplay()
{
    drawBackground();
    drawTerrain();
    drawTurrets();
    drawPlayers();
    drawBullets();
    drawPlayersHUD();
    drawGameOverUI();
    // drawDebug(); // debug
}

void Game::drawPlayersHUD()
{
    sf::Vector2f viewSize = window.getView().getSize();
    const float screenWidth = viewSize.x;
    const float sectionWidth = screenWidth / 4.f;
    const float hudHeight = 60.f;
    const float margin = 8.f;

    int index = 0;

    for (const auto &[id, player] : client.allPlayers)
    {
        if (index >= 4)
            break;

        const sf::Color playerColor = PlayerColors[player.id];
        float baseX = player.id * sectionWidth;

        // Fond semi-transparent (optionnel)
        sf::RectangleShape background({sectionWidth, hudHeight});
        background.setPosition({baseX, 0.f});
        background.setFillColor(sf::Color(0, 0, 0, 120));
        window.draw(background);

        // Texte joueur
        sf::Text text(Config::Get().font);
        text.setCharacterSize(16);
        text.setFillColor(playerColor);

        text.setString(
            "Player " + std::to_string(id) +
            "\nScore: " + std::to_string(static_cast<int>(player.score)));
        text.setPosition({baseX + margin, margin});
        window.draw(text);

        // Barre de PV
        const float maxPV = player.maxPv; // valeur max connue
        float pvRatio = std::max(0.f, player.pv) / maxPV;

        sf::RectangleShape pvBg({sectionWidth - 2 * margin, 8.f});
        pvBg.setPosition({baseX + margin, hudHeight - 16.f});
        pvBg.setFillColor(sf::Color(80, 80, 80));
        window.draw(pvBg);

        sf::RectangleShape pvBar({pvBg.getSize().x * pvRatio, 8.f});
        pvBar.setPosition(pvBg.getPosition());
        pvBar.setFillColor(player.invulnerable ? sf::Color::Yellow : playerColor);
        window.draw(pvBar);

        ++index;
    }
}

void Game::drawBackground()
{
    sf::RenderStates states_1, states_2;
    states_1.texture = &Config::Get().bckgTextureBack; // bckgTextureBack;
    states_2.texture = &Config::Get().bckgTextureFront;

    window.draw(backgroundVA_1, states_1);
    window.draw(backgroundVA_2, states_2);
}

void Game::drawTerrain()
{
    sf::Sprite sprite(Config::Get().blockTexture);
    // sprite.setTexture(blocksTexture);

    for (const auto &seg : client.terrain.segments)
    {
        for (const auto &b : seg.blocks)
        {
            float screenX = b.rect.position.x + seg.startX - client.terrain.worldX;

            if (screenX + b.rect.size.x < 0 ||
                screenX > Config::Get().windowSize.x)
                continue;

            sprite.setPosition({screenX, b.rect.position.y});
            sprite.setTextureRect(client.terrain.getTextureRect(b.visual));

            window.draw(sprite);
        }
    }
}

void Game::drawTurrets()
{
    sf::Sprite sprite(Config::Get().turretTexture);

    int cellX = Config::Get().turretTexture.getSize().x / 6;
    int cellY = Config::Get().turretTexture.getSize().y / 2;

    sprite.setScale({3.f, 3.f});
    sprite.setOrigin({cellX * 0.5f, cellY * 0.5f});

    for (const auto &[id, turret] : client.allTurrets)
    {
        if (!turret.active)
            continue;

        // Ici on peut choisir la texture selon le type ou orientation
        // Exemple simple : turrets au sol
        sprite.setTextureRect(sf::IntRect({cellX * 1, cellY * 0}, {cellX, cellY}));

        // Position à l’écran : position monde - scroll
        float screenX = turret.position.x - client.terrain.worldX;
        float screenY = turret.position.y;

        sprite.setPosition({screenX, screenY});

        window.draw(sprite);
    }
}

void Game::drawPlayers()
{
    sf::RenderStates states;
    states.texture = &Config::Get().playerTexture;
    window.draw(playersVA, states);
}

void Game::drawBullets()
{
    if (bulletsVA.getVertexCount() == 0)
        return;

    sf::RenderStates states;
    states.texture = nullptr; //&Config::Get().texture; // même texture que tes sprites joueurs, ou autre texture pour bullets
    window.draw(bulletsVA, states);
}

void Game::drawGameOverUI()
{
    auto it = client.allPlayers.find(Config::Get().playerId);
    if (it == client.allPlayers.end())
        return;

    RemotePlayer &p = it->second;

    if (p.state != RemotePlayerState::GameOver)
        return;

    sf::RectangleShape overlay(window.getView().getSize());
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    sf::Text text(
        Config::Get().font,
        "GAME OVER\nPress ENTER to rejoin",
        32);
    text.setFillColor(sf::Color::White);
    text.setPosition(
        {window.getView().getSize().x * 0.5f - 200.f,
         window.getView().getSize().y * 0.5f - 60.f});

    window.draw(text);
}
