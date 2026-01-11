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
    drawEnemies();
    drawPlayers();
    Bullet::drawBullets(window);
    Bonus::drawBonuses(window);
    Explosion::draw(client, window);
    drawPlayersHUD();
    drawGameOverUI();
}

void Game::drawEnemies()
{
    std::vector<Enemy> enemyCopy;
    {
        std::lock_guard<std::mutex> lock(client.enemiesMutex);
        for (auto &[id, e] : client.allEnemies)
            enemyCopy.push_back(e);
    }

    for (auto &e : enemyCopy)
        e.draw(client, window);
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
        float baseX = index * sectionWidth;

        sf::RectangleShape background({sectionWidth, hudHeight});
        background.setPosition({baseX, 0.f});
        background.setFillColor(sf::Color(0, 0, 0, 120));
        window.draw(background);

        sf::Text text(Config::Get().font);
        text.setCharacterSize(16);
        text.setFillColor(playerColor);
        text.setString("Player " + std::to_string(player.id) +
                       "\nScore: " + std::to_string(static_cast<int>(player.score)));
        text.setPosition({baseX + margin, 0.f});
        window.draw(text);

        sf::Sprite rocket(Config::Get().rocketTexture);
        rocket.scale({.75f, .75f});
        rocket.setRotation(sf::degrees(-45.f));
        rocket.setPosition({50 + baseX + margin + text.getGlobalBounds().size.x, margin + rocket.getGlobalBounds().size.y});

        window.draw(rocket);

        sf::Text rocketCountText(Config::Get().font);
        rocketCountText.setCharacterSize(16);
        rocketCountText.setFillColor(playerColor);
        rocketCountText.setString(std::to_string(player.nbRocket));
        rocketCountText.setPosition({rocket.getPosition().x + rocket.getGlobalBounds().size.x + 4.f,
                                     rocket.getPosition().y - rocketCountText.getGlobalBounds().size.y});
        window.draw(rocketCountText);

        const float maxPV = player.maxPv;
        float pvRatio = std::max(0.f, player.pv) / maxPV;

        sf::RectangleShape pvBg({sectionWidth - 2 * margin, 10.f});
        pvBg.setPosition({baseX + margin, hudHeight - 18.f});
        pvBg.setFillColor(sf::Color(80, 80, 80));
        window.draw(pvBg);

        sf::RectangleShape pvBar({pvBg.getSize().x * pvRatio, 10.f});
        pvBar.setPosition(pvBg.getPosition());
        pvBar.setFillColor(player.invulnerable ? sf::Color::Yellow : playerColor);
        window.draw(pvBar);

        ++index;
    }
}

void Game::drawBackground()
{
    sf::RenderStates states_1, states_2;
    states_1.texture = &Config::Get().bckgTextureBack;
    states_2.texture = &Config::Get().bckgTextureFront;

    window.draw(backgroundVA_1, states_1);
    window.draw(backgroundVA_2, states_2);
}

void Game::drawTerrain()
{
    for (const auto &seg : client.terrain.segments)
    {
        if (seg.startX - client.targetWorldX > window.getSize().x ||
            seg.blocks.size() * seg.blocks[0].rect.size.x + seg.startX < 0)
            continue;
        for (const auto &blk : seg.blocks)
        {
            if (seg.startX + blk.rect.position.x + blk.rect.size.x - client.targetWorldX < 0)
                continue;
            const sf::Texture &tex = ClientTileRegistry::getTexture(blk.tileId);

            sf::Sprite spr(tex);
            spr.setTextureRect(ClientTileRegistry::getTextureRect(blk.visual, tex));
            spr.setPosition({seg.startX + blk.rect.position.x - client.targetWorldX, blk.rect.position.y});

            window.draw(spr);
        }
    }
}

void Game::drawPlayers()
{
    sf::RenderStates states;
    states.texture = &Config::Get().playerTexture;
    window.draw(playersVA, states);
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
