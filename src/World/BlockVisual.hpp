
#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <string>

enum class BlockVisual
{
    Left,
    Middle,
    Right

};

struct TerrainBlock
{
    sf::FloatRect rect;
    BlockVisual visual;
    uint16_t tileId;
};

struct TerrainBlockClient
{
    sf::FloatRect rect;
    BlockVisual visual;
    std::string tileId;
    bool hasTurret = false;
};