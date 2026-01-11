#include "Terrain.hpp"

Terrain::Terrain() : nextSegmentX(0.f) {}

void Terrain::init(uint32_t seed)
{
    rng.seed(seed);

    segments.clear();
    nextSegmentX = 0.f;
    lookahead = 3.f * Config::Get().windowSize.x;
    cleanupMargin = 2.f * Config::Get().windowSize.x;
}
