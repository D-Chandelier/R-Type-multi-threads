#include "Game.hpp"

void Game::update(float dt)
{
    if (currentMenu && state != GameState::IN_GAME)
    {
        currentMenu->update(dt, window);
        menuBackground.update(dt);
        handleMenuAction(); // <-- important
    }
    else if (state == GameState::IN_GAME)
    {
        updateGameplay(dt); // <-- ta future logique in-game
    }
}

void Game::updateGameplay(float dt)
{
    updateBackgrounds();
    updatePlayers();
    updateBullets(dt);
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

    // offset horizontal ou vertical
    float offsetX = background_1_OffsetX; // déjà calculé
    float offsetY = background_1_OffsetY; // pour vertical

    // Scroll horizontal + vertical, en boucle grâce à setRepeated(true)
    backgroundVA_1[0].texCoords = {offsetX, offsetY};
    backgroundVA_1[1].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY};
    backgroundVA_1[2].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_1[3].texCoords = {offsetX, offsetY};
    backgroundVA_1[4].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_1[5].texCoords = {offsetX, offsetY + static_cast<float>(winSize.y)};

    offsetX = background_2_OffsetX; // déjà calculé
    offsetY = background_2_OffsetY; // pour vertical
    backgroundVA_2[0].texCoords = {offsetX, offsetY};
    backgroundVA_2[1].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY};
    backgroundVA_2[2].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_2[3].texCoords = {offsetX, offsetY};
    backgroundVA_2[4].texCoords = {offsetX + static_cast<float>(winSize.x), offsetY + static_cast<float>(winSize.y)};
    backgroundVA_2[5].texCoords = {offsetX, offsetY + static_cast<float>(winSize.y)};

    sf::Color alphaColor(255, 255, 255, 128); // 50% transparent

    for (size_t i = 0; i < backgroundVA_2.getVertexCount(); ++i)
        backgroundVA_2[i].color = alphaColor; // 50% transparent
}

// void Game::updateTerrain()
// {
//     // Interpolation pour smooth scrolling
//     double delta = localTimeNow() - client.serverGameTime;
//     float alpha = static_cast<float>(delta / (1.0f / Config::Get().frameRate));
//     alpha = std::clamp(alpha, 0.f, 1.f);

//     client.terrain.worldX += (client.targetWorldX - client.terrain.worldX) * alpha;

//     // Nettoyage segments déjà passés (optionnel)
//     while (!client.terrain.segments.empty() &&
//            client.terrain.segments.front().startX + SEGMENT_WIDTH < client.terrain.worldX - 100.f)
//     {
//         client.terrain.segments.pop_front();
//     }
// }

void Game::updatePlayers()
{
    int i = 0;
    int cellWidth, cellHeight;

    playersVA.resize(client.allPlayers.size() * 6);

    for (auto &[id, p] : client.allPlayers)
    {
        bool visible = true;
        if (p.invulnerable)
        {
            double t = Utils::localTimeNow() - p.respawnTime;
            visible = static_cast<int>(t * 10) % 2 == 0; // 10 Hz
        }
        if (!visible)
            continue; // ne pas dessiner ce frame

        // Temps écoulé depuis la dernière position serveur
        double delta = Utils::localTimeNow() - p.lastUpdateTime;
        // Interpolation simple
        float alpha = static_cast<float>(delta / (2.f / Config::Get().frameRate)); // delta / tick serveur (xx ms)
        alpha = std::clamp(alpha, 0.f, 1.f);

        // p.position = p.serverPosition; // reset avant lerp
        p.position += (p.serverPosition - p.position) * alpha;

        cellWidth = Config::Get().playerArea.size.x;
        cellHeight = Config::Get().playerArea.size.y;

        sf::Vector2f origin = p.position; // getOrigine();
        sf::Vector2f scale = Config::Get().playerScale;
        float w = p.getBounds().size.x; // static_cast<float>(cellWidth) * scale.x;
        float h = p.getBounds().size.y; // static_cast<float>(cellHeight) * scale.y;

        float x = p.getBounds().position.x; // p.position.x; //(p.position.x - origin.x * scale.x);
        float y = p.getBounds().position.y; //(p.position.y - origin.y * scale.y);

        // positions du quad (2 triangles)
        playersVA[i * 6 + 0].position = {x, y};
        playersVA[i * 6 + 1].position = {x + w, y};
        playersVA[i * 6 + 2].position = {x + w, y + h};

        playersVA[i * 6 + 3].position = {x, y};
        playersVA[i * 6 + 4].position = {x + w, y + h};
        playersVA[i * 6 + 5].position = {x, y + h};

        // texCoords selon sprite dans spritesheet
        float tx = 2 * cellWidth;
        float ty = id * cellHeight;
        playersVA[i * 6 + 0].texCoords = {tx, ty};
        playersVA[i * 6 + 1].texCoords = {tx + cellWidth, ty};
        playersVA[i * 6 + 2].texCoords = {tx + cellWidth, ty + cellHeight};
        playersVA[i * 6 + 3].texCoords = {tx, ty};
        playersVA[i * 6 + 4].texCoords = {tx + cellWidth, ty + cellHeight};
        playersVA[i * 6 + 5].texCoords = {tx, ty + cellHeight};

        i++;
    }
}

void Game::updateBullets(float dt)
{
    // Mise à jour logique
    for (auto &[id, b] : client.allBullets)
    {
        b.update(dt);
    }

    // Nettoyage
    std::erase_if(client.allBullets,
                  [](const auto &it)
                  { return !it.second.active; });

    // Rebuild Bullet VA
    bulletsVA.clear();
    bulletsVA.setPrimitiveType(sf::PrimitiveType::Triangles);

    for (const auto &[id, b] : client.allBullets)
    {
        float w = 10.f;
        float h = 5.f;
        float x = b.position.x - w / 2.f;
        float y = b.position.y - h / 2.f;

        sf::Color color = sf::Color::Cyan;

        bulletsVA.append({{x, y}, color});
        bulletsVA.append({{x + w, y}, color});
        bulletsVA.append({{x + w, y + h}, color});
        bulletsVA.append({{x, y}, color});
        bulletsVA.append({{x + w, y + h}, color});
        bulletsVA.append({{x, y + h}, color});
    }
}
