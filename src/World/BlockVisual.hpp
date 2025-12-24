
#pragma once
#include <SFML/Graphics/Rect.hpp>

enum class BlockVisual
{
    CeilingLeft,
    CeilingMid,
    CeilingRight,

    GroundTopLeft,
    GroundTopMid,
    GroundTopRight,

    GroundFillLeft,
    GroundFillMid,
    GroundFillRight,

    BridgeLeft,
    BridgeMid,
    BridgeRight
};

struct TerrainBlock
{
    sf::FloatRect rect;
    BlockVisual visual;
    bool hasTurret = false;
};
