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
    // drawDebug(); // debug
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

    for (const auto &seg : client.terrain.segments)
    {
        for (const auto &block : seg.blocks)
        {
            if (!block.hasTurret)
                continue;

            const bool isCeiling =
                block.visual == BlockVisual::CeilingLeft ||
                block.visual == BlockVisual::CeilingMid ||
                block.visual == BlockVisual::CeilingRight;

            // position MONDE
            float worldX =
                seg.startX +
                block.rect.position.x +
                block.rect.size.x * 0.5f;

            float worldY =
                isCeiling
                    ? block.rect.position.y + block.rect.size.y
                    : block.rect.position.y;

            // rotation

            if (isCeiling)
            {
                sprite.setTextureRect(sf::IntRect({cellX * 1, cellY * 1}, {cellX, cellY}));
                sprite.setPosition({worldX - client.terrain.worldX, worldY});
            }
            else
            {
                sprite.setTextureRect(sf::IntRect({cellX * 1, cellY * 0}, {cellX, cellY}));
                sprite.setPosition({worldX - client.terrain.worldX, worldY - cellY * sprite.getScale().y / 2});
            }

            window.draw(sprite);
        }
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
