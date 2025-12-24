#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include "BlockVisual.hpp"
#include "../Entities/Turret.hpp"

constexpr float SEGMENT_WIDTH = 512.f;
constexpr float GROUND_HEIGHT = 128.f;
constexpr float TILE = 64.f;

class Terrain;

enum class SegmentType
{
    Flat,
    Hole,
    Corridor,
    TurretZone
};

struct TerrainSegment
{
    float startX;
    SegmentType type;
    std::vector<TerrainBlock> blocks; // zones solides
    std::vector<Turret> turrets;

    bool isOffScreen() const
    {
        return startX + SEGMENT_WIDTH < 0.f;
    }
};

namespace Segments
{
    TerrainSegment generateNextSegment(Terrain &terrain);
}