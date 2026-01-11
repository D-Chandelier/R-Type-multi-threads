#include "Game.hpp"

void Game::update(float dt)
{
    if (currentMenu && state != GameState::IN_GAME)
    {
        currentMenu->update(dt, window);
        menuBackground.update(dt);
        handleMenuAction();
    }
    else if (state == GameState::IN_GAME)
    {
        updateGameplay(dt);
    }
}

void Game::updateGameplay(float dt)
{
    updateBackgrounds();
    updatePlayers();
    Bullet::updateBulletsClient(client, dt);
    Bonus::updateBonusesClient(client, dt);
    Explosion::updateExplosionsClient(client, dt);
}

void Game::updateBackgrounds()
{
    float texW = static_cast<float>(Config::Get().bckgTextureBack.getSize().x);
    float texH = static_cast<float>(Config::Get().bckgTextureFront.getSize().y);
    sf::Vector2f winSize = {static_cast<float>(Config::Get().windowSize.x), static_cast<float>(Config::Get().windowSize.y)};

    double gameTime = Utils::localTimeNow() + client.serverTimeOffset;

    background_1_OffsetX = std::fmod(
        static_cast<float>(gameTime) * client.backgroundScrollSpeed,
        texH);
    background_2_OffsetX = std::fmod(
        static_cast<float>(gameTime) * client.backgroundScrollSpeed * 1.5f,
        texH);

    float offsetX = background_1_OffsetX;
    float offsetY = background_1_OffsetY;

    backgroundVA_1[0].texCoords = {offsetX, offsetY};
    backgroundVA_1[1].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY};
    backgroundVA_1[2].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_1[3].texCoords = {offsetX, offsetY};
    backgroundVA_1[4].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_1[5].texCoords = {offsetX, offsetY + static_cast<float>(winSize.y)};

    offsetX = background_2_OffsetX;
    offsetY = background_2_OffsetY;
    backgroundVA_2[0].texCoords = {offsetX, offsetY};
    backgroundVA_2[1].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY};
    backgroundVA_2[2].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_2[3].texCoords = {offsetX, offsetY};
    backgroundVA_2[4].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_2[5].texCoords = {offsetX, offsetY + static_cast<float>(winSize.y)};

    sf::Color alphaColor(255, 255, 255, 128);

    for (size_t i = 0; i < backgroundVA_2.getVertexCount(); ++i)
        backgroundVA_2[i].color = alphaColor;
}

void Game::updatePlayers()
{
    playersVA.clear();

    for (auto &[id, p] : client.allPlayers)
    {
        if (p.pv <= 0.f)
        {
            p.state = RemotePlayerState::GameOver;
            continue;
        }
        p.state = RemotePlayerState::Playing;

        bool visible = true;

        if (p.invulnerable)
        {
            double t = Utils::localTimeNow() - p.respawnTime;
            visible = static_cast<int>(t * 10) % 2 == 0;
        }

        if (!visible)
            continue;

        double delta = Utils::localTimeNow() - p.lastUpdateTime;
        float alpha = static_cast<float>(delta / (1.f / Config::Get().frameRate));
        alpha = std::clamp(alpha, 0.f, 1.f);

        p.position += (p.serverPosition - p.position) * alpha;

        float w = p.getBounds().size.x;
        float h = p.getBounds().size.y;
        float x = p.getBounds().position.x;
        float y = p.getBounds().position.y;

        sf::Vertex quad[6];

        quad[0].position = {x, y};
        quad[1].position = {x + w, y};
        quad[2].position = {x + w, y + h};
        quad[3].position = {x, y};
        quad[4].position = {x + w, y + h};
        quad[5].position = {x, y + h};

        float cellW = Config::Get().playerArea.size.x;
        float cellH = Config::Get().playerArea.size.y;
        float tx = 2 * cellW;
        float ty = id * cellH;

        quad[0].texCoords = {tx, ty};
        quad[1].texCoords = {tx + cellW, ty};
        quad[2].texCoords = {tx + cellW, ty + cellH};
        quad[3].texCoords = {tx, ty};
        quad[4].texCoords = {tx + cellW, ty + cellH};
        quad[5].texCoords = {tx, ty + cellH};

        for (int i = 0; i < 6; ++i)
            playersVA.append(quad[i]);
    }
}
