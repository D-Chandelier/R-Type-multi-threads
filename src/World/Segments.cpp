#include "Segments.hpp"
#include "Terrain.hpp"
#include "../Network/Server/Server.hpp"

SegmentType Segment::segmentTypeFromString(const std::string &s)
{
    if (s == "none")
        return SegmentType::None;
    if (s == "full")
        return SegmentType::Full;
    if (s == "hole")
        return SegmentType::Hole;
    if (s == "left")
        return SegmentType::Left;
    if (s == "right")
        return SegmentType::Right;
    if (s == "middle")
        return SegmentType::Middle;

    throw std::runtime_error("Unknown SegmentType: " + s);
}

TileType Segment::tileTypeFromString(const std::string &s)
{
    if (s == "ground")
        return TileType::Ground;
    if (s == "stone")
        return TileType::Stone;
    if (s == "metal")
        return TileType::Metal;
    if (s == "plateform")
        return TileType::Platform;
    if (s == "bridge")
        return TileType::Bridge;
    if (s == "ceiling")
        return TileType::Ceiling;

    return TileType::Default;
}

void Segment::updateSegment(Server &s)
{
    LevelRuntime &lvl = s.runtimeLevel;
    const LevelDesc *desc = lvl.desc;

    while (lvl.nextSegmentIndex < desc->segments.size())
    {
        const auto &segDesc = desc->segments[lvl.nextSegmentIndex];

        if (segDesc.atX > s.worldX + s.lookahead)
            break;

        TerrainSegment seg = buildSegment(segDesc);

        s.terrain.segments.push_back(seg);
        s.terrain.nextSegmentX = seg.startX + desc->segmentWidth;

        s.packetBroadcastSegment(seg);

        lvl.nextSegmentIndex++;
    }
}

TerrainSegment Segment::buildSegment(const LevelSegmentDesc &desc)
{
    TerrainSegment seg;
    seg.startX = desc.atX;

    const int tilesX = static_cast<int>(LevelRegistry::current()->segmentWidth / LevelRegistry::current()->tileSize.x);
    const int tierTiles = tilesX / 3;

    for (const ModuleDesc &mod : desc.modules)
    {
        float moduleY =
            Config::Get().windowSize.y - LevelRegistry::current()->tileSize.y * (mod.level + mod.height);

        for (int y = 0; y < mod.height; ++y)
        {
            if (mod.type == SegmentType::Full)
            {
                for (int x = 0; x < tilesX; ++x)
                {
                    seg.blocks.push_back({sf::FloatRect{
                                              {x * LevelRegistry::current()->tileSize.x, moduleY + y * LevelRegistry::current()->tileSize.y},
                                              {LevelRegistry::current()->tileSize.x, LevelRegistry::current()->tileSize.y}},
                                          getGroundVisual(x, tilesX),
                                          mod.tileId});
                }
                continue;
            }

            TierMask mask = getTierMask(mod.type);

            for (int x = 0; x < tierTiles; ++x)
            {
                if (mask.left)
                {
                    seg.blocks.push_back({sf::FloatRect{
                                              {x * LevelRegistry::current()->tileSize.x, moduleY + y * LevelRegistry::current()->tileSize.y},
                                              {LevelRegistry::current()->tileSize.x, LevelRegistry::current()->tileSize.y}},
                                          getGroundVisual(x, tierTiles),
                                          mod.tileId});
                }

                if (mask.mid)
                {
                    seg.blocks.push_back({sf::FloatRect{
                                              {(tierTiles + x) * LevelRegistry::current()->tileSize.x, moduleY + y * LevelRegistry::current()->tileSize.y},
                                              {LevelRegistry::current()->tileSize.x, LevelRegistry::current()->tileSize.y}},
                                          getGroundVisual(x, tierTiles),
                                          mod.tileId});
                }

                if (mask.right)
                {
                    seg.blocks.push_back({sf::FloatRect{
                                              {(2 * tierTiles + x) * LevelRegistry::current()->tileSize.x, moduleY + y * LevelRegistry::current()->tileSize.y},
                                              {LevelRegistry::current()->tileSize.x, LevelRegistry::current()->tileSize.y}},
                                          getGroundVisual(x, tierTiles),
                                          mod.tileId});
                }
            }
        }
    }

    return seg;
}

TierMask Segment::getTierMask(SegmentType type)
{
    switch (type)
    {
    case SegmentType::Full:
        return {true, true, true};

    case SegmentType::Left:
        return {true, false, false};

    case SegmentType::Middle:
        return {false, true, false};

    case SegmentType::Right:
        return {false, false, true};

    case SegmentType::Hole:
        return {true, false, true};

    case SegmentType::None:
    default:
        return {};
    }
}

BlockVisual Segment::getGroundVisual(int x, int tierTiles)
{
    if (x == 0)
        return BlockVisual::Left;
    if (x == tierTiles - 1)
        return BlockVisual::Right;
    return BlockVisual::Middle;
}