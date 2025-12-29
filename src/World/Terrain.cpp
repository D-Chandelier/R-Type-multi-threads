#include "Terrain.hpp"

Terrain::Terrain() : nextSegmentX(0.f) {}

void Terrain::init(uint32_t seed)
{
    rng.seed(seed);

    segments.clear();
    nextSegmentX = 0.f;
    lookahead = 3.f * Config::Get().windowSize.x;
    cleanupMargin = 2.f * Config::Get().windowSize.x;
    // // Générer juste assez pour le début
    // generateNextSegment();

    // generateNextSegment();
}

sf::IntRect Terrain::getTextureRect(BlockVisual v)
{
    constexpr int TILE = 64;

    switch (v)
    {
    case BlockVisual::CeilingLeft:
        return {{0 * TILE, 0 * TILE}, {TILE, TILE}};
    case BlockVisual::CeilingMid:
        return {{1 * TILE, 0 * TILE}, {TILE, TILE}};
    case BlockVisual::CeilingRight:
        return {{2 * TILE, 0 * TILE}, {TILE, TILE}};
    case BlockVisual::GroundTopLeft:
        return {{0 * TILE, 1 * TILE}, {TILE, TILE}};
    case BlockVisual::GroundTopMid:
        return {{1 * TILE, 1 * TILE}, {TILE, TILE}};
    case BlockVisual::GroundTopRight:
        return {{2 * TILE, 1 * TILE}, {TILE, TILE}};
    case BlockVisual::GroundFillLeft:
        return {{0 * TILE, 2 * TILE}, {TILE, TILE}};
    case BlockVisual::GroundFillMid:
        return {{1 * TILE, 2 * TILE}, {TILE, TILE}};
    case BlockVisual::GroundFillRight:
        return {{2 * TILE, 2 * TILE}, {TILE, TILE}};
    case BlockVisual::BridgeLeft:
        return {{0 * TILE, 0 * TILE}, {TILE, TILE}};
    case BlockVisual::BridgeMid:
        return {{1 * TILE, 0 * TILE}, {TILE, TILE}};
    case BlockVisual::BridgeRight:
        return {{2 * TILE, 0 * TILE}, {TILE, TILE}};
    }

    return {{0, 0}, {TILE, TILE}};
};
