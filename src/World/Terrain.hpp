#pragma once
#include <SFML/Graphics.hpp>
#include "../Core/Config.hpp"
#include "Segments.hpp"
#include <deque>
#include <random>
#include "BlockVisual.hpp"

class Terrain
{
public:
    Terrain();

    void init(uint32_t seed);
    void update(float worldX);
    void updateCollision(float worldXServer);         // update segments côté collision
    void updateDraw(float targetWorldX, float alpha); // update worldX pour dessin
    void draw(sf::RenderWindow &win);
    bool collides(const sf::FloatRect &box) const;

    sf::IntRect getTextureRect(BlockVisual v);

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
    void generateNextSegment();
    void generateNextSegmentAt(float startX);
};
