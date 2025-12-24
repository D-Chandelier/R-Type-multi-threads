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

// void Terrain::generateNextSegment()
// {
//     TerrainSegment seg;
//     seg.startX = nextSegmentX;

//     std::uniform_int_distribution<int> dist(0, 3);
//     seg.type = static_cast<SegmentType>(dist(rng));

//     float groundY = Config::Get().windowSize.y - GROUND_HEIGHT;

//     switch (seg.type)
//     {
//     case SegmentType::Flat:
//         seg.blocks.emplace_back(sf::FloatRect{{0.f, groundY}, {SEGMENT_WIDTH, GROUND_HEIGHT}});
//         break;

//     case SegmentType::Hole:
//     {
//         float holeWidth = SEGMENT_WIDTH * 0.4f;
//         seg.blocks.emplace_back(sf::FloatRect{{0.f, groundY}, {(SEGMENT_WIDTH - holeWidth) / 2.f, GROUND_HEIGHT}});
//         seg.blocks.emplace_back(sf::FloatRect{{(SEGMENT_WIDTH + holeWidth) / 2.f, groundY}, {(SEGMENT_WIDTH - holeWidth) / 2.f, GROUND_HEIGHT}});
//         break;
//     }

//     case SegmentType::Corridor:
//         seg.blocks.emplace_back(sf::FloatRect{{0.f, groundY}, {SEGMENT_WIDTH, GROUND_HEIGHT}});
//         seg.blocks.emplace_back(sf::FloatRect{{0.f, 0.f}, {SEGMENT_WIDTH, 80.f}});
//         break;

//     case SegmentType::TurretZone:
//         seg.blocks.emplace_back(sf::FloatRect{{0.f, groundY}, {SEGMENT_WIDTH, GROUND_HEIGHT}});
//         break;
//     }

//     segments.push_back(seg);
//     nextSegmentX += SEGMENT_WIDTH;
// }

// void Terrain::update(float worldX)
// {
//     // Générer suffisamment loin devant le monde
//     while (nextSegmentX < worldX + lookahead)
//         generateNextSegment();

//     // Nettoyage optionnel
//     while (!segments.empty() &&
//            segments.front().startX + SEGMENT_WIDTH < worldX - cleanupMargin)
//     {
//         segments.pop_front();
//     }
// }

void Terrain::generateNextSegmentAt(float startX)
{
    TerrainSegment seg;
    seg.startX = startX;

    static int index = 0;
    seg.type = static_cast<SegmentType>(index % 4);
    index++;

    float groundY = Config::Get().windowSize.y - GROUND_HEIGHT;

    switch (seg.type)
    {
    case SegmentType::Flat:
        seg.blocks.emplace_back(sf::FloatRect{{0.f, groundY}, {SEGMENT_WIDTH, GROUND_HEIGHT}});
        break;
    case SegmentType::Hole:
    {
        float holeWidth = SEGMENT_WIDTH * 0.4f;
        seg.blocks.emplace_back(sf::FloatRect{{0.f, groundY}, {(SEGMENT_WIDTH - holeWidth) / 2.f, GROUND_HEIGHT}});
        seg.blocks.emplace_back(sf::FloatRect{{(SEGMENT_WIDTH + holeWidth) / 2.f, groundY}, {(SEGMENT_WIDTH - holeWidth) / 2.f, GROUND_HEIGHT}});
        break;
    }
    case SegmentType::Corridor:
        seg.blocks.emplace_back(sf::FloatRect{{0.f, groundY}, {SEGMENT_WIDTH, GROUND_HEIGHT}});
        seg.blocks.emplace_back(sf::FloatRect{{0.f, 0.f}, {SEGMENT_WIDTH, 80.f}});
        break;
    case SegmentType::TurretZone:
        seg.blocks.emplace_back(sf::FloatRect{{0.f, groundY}, {SEGMENT_WIDTH, GROUND_HEIGHT}});
        break;
    }

    segments.push_back(seg);
}

// void Terrain::updateCollision(float worldXServer)
// {
//     collisionX = worldXServer;

//     // Générer suffisamment loin devant
//     while (nextSegmentX < collisionX + lookahead)
//         generateNextSegment();

//     // Nettoyer derrière
//     while (!segments.empty() && segments.front().startX + SEGMENT_WIDTH < collisionX - cleanupMargin)
//         segments.pop_front();
// }

bool Terrain::collides(const sf::FloatRect &box) const
{
    for (const auto &seg : segments)
    {
        for (const auto &block : seg.blocks)
        {
            sf::FloatRect worldBlock = block.rect;
            worldBlock.position.x += seg.startX; // position absolue

            // SFML3 : findIntersection
            if (worldBlock.findIntersection(box))
                return true;
        }
    }
    return false;
}

void Terrain::updateDraw(float targetWorldX, float alpha)
{
    // Interpolation pour le rendu fluide
    worldX += (targetWorldX - worldX) * alpha;
}

void Terrain::draw(sf::RenderWindow &win)
{
    sf::RectangleShape r;
    r.setFillColor(sf::Color(80, 80, 80));

    for (const auto &seg : segments)
    {
        for (const auto &block : seg.blocks)
        {
            float screenX = block.rect.position.x - worldX;
            if (screenX + block.rect.size.x < 0 || screenX > Config::Get().windowSize.x)
                continue;

            r.setPosition({screenX, block.rect.position.y});
            r.setSize(block.rect.size);
            win.draw(r);
        }
    }
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
