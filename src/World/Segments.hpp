#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include "BlockVisual.hpp"
#include "../Entities/Enemies.hpp"
#include "Level.hpp"

class Terrain;
class Server;

enum class SegmentType
{
    None,
    Full,
    Hole,
    Left,
    Right,
    Middle
};

enum class TileType
{
    Default,
    Stone,
    Ground,
    Metal,
    Platform,
    Bridge,
    Ceiling

};

struct ModuleDesc
{
    SegmentType type = SegmentType::Full;
    int level = 0;
    int height = 1;
    uint16_t tileId;
};

struct TierMask
{
    bool left = false;
    bool mid = false;
    bool right = false;
};

struct LevelSegmentDesc
{
    float atX = 0.f;
    std::string tag;

    std::vector<ModuleDesc> modules;

    std::vector<TurretPlacement> turrets;
};

struct TerrainSegment
{
    float startX;
    SegmentType type;
    std::vector<TerrainBlock> blocks;
};

namespace Segment
{

    SegmentType segmentTypeFromString(const std::string &s);
    TileType tileTypeFromString(const std::string &s);
    void updateSegment(Server &s);
    TerrainSegment buildSegment(const LevelSegmentDesc &desc);
    BlockVisual getGroundVisual(int x, int tierTiles);
    TierMask getTierMask(SegmentType type);
}