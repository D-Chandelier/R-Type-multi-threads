#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <vector>

constexpr float SEGMENT_WIDTH = 512.f;
constexpr float GROUND_HEIGHT = 64.f;
constexpr float LEVEL_SCROLL_SPEED = 180.f;

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
    std::vector<sf::FloatRect> blocks; // zones solides

    bool isOffScreen() const
    {
        return startX + SEGMENT_WIDTH < 0.f;
    }
};
