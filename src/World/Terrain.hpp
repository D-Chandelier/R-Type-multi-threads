#pragma once
#include <SFML/Graphics.hpp>
#include <deque>
#include <random>
#include "../Core/Config.hpp"
#include "Segments.hpp"
#include "BlockVisual.hpp"

class Terrain
{
public:
    Terrain();

    void init(uint32_t seed);

    uint32_t levelSeed;
    uint32_t serverTick;
    float backgroundScrollSpeed;
    float lookahead;
    float cleanupMargin;
    float worldX;
    float collisionX;
    std::deque<TerrainSegment> segments;
    float nextSegmentX = 0.f;
    std::mt19937 rng;

private:
};
