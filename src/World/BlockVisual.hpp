
#pragma once
#include <SFML/Graphics/Rect.hpp>

class BlockVisual
{
    CeilingLeft,
        CeilingMid,
        CeilingRight,

        GroundTopLeft,
        GroundTopMid,
        GroundTopRight,

        GroundFillLeft,
        GroundFillMid,
        GroundFillRight
};
struct TerrainBlock
{
    sf::FloatRect rect;
    BlockVisual visual;
};
