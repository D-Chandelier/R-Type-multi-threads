#include "Level.hpp"
#include <iostream>

std::unordered_map<std::string, LevelDesc> LevelRegistry::levels;
const LevelDesc *LevelRegistry::currentLevel = nullptr;

void LevelRegistry::loadFromFile(const std::string &path)
{
    YAML::Node root = YAML::LoadFile(path);
    if (!root || !root.IsMap())
        throw std::runtime_error("Level YAML root must be a map");

    LevelDesc lvl;

    lvl.id = root["id"] ? root["id"].as<std::string>() : throw std::runtime_error("Level missing 'id'");
    lvl.scrollSpeed = root["scrollSpeed"] ? root["scrollSpeed"].as<float>() : 180.f;
    lvl.seed = root["seed"] ? root["seed"].as<uint32_t>() : 0;
    lvl.tileSize.x = root["tile_width"] ? root["tile_width"].as<float>() : 64.f;
    lvl.tileSize.y = root["tile_height"] ? root["tile_height"].as<float>() : 64.f;
    lvl.segmentWidth = root["segment_width"] ? root["segment_width"].as<float>() : 1152.f;

    const YAML::Node &segs = root["segments"];
    if (!segs || !segs.IsSequence())
        throw std::runtime_error("Level '" + lvl.id + "' must contain 'segments' as a sequence");

    for (const auto &segNode : segs)
    {
        LevelSegmentDesc segDesc;
        segDesc.atX = segNode["atX"] ? segNode["atX"].as<float>() : 0.f;
        segDesc.tag = segNode["tag"] ? segNode["tag"].as<std::string>() : "";

        const YAML::Node &modules = segNode["modules"];
        if (modules && modules.IsSequence())
        {
            for (const auto &m : modules)
            {
                ModuleDesc mod;
                mod.type = m["type"] ? Segment::segmentTypeFromString(m["type"].as<std::string>()) : SegmentType::None;
                mod.level = m["level"] ? m["level"].as<int>() : 0;
                mod.height = m["height"] ? m["height"].as<int>() : 1;
                mod.tileId = m["tile"] ? ServerTileRegistry::getId(m["tile"].as<std::string>()) : ServerTileRegistry::getId("default");
                segDesc.modules.push_back(mod);
            }
        }

        lvl.segments.push_back(segDesc);
    }

    const YAML::Node &enems = root["enemies"];
    if (enems && enems.IsSequence())
    {
        for (const auto &eNode : enems)
        {
            EnemySpawnDesc e;
            e.atX = eNode["atX"] ? eNode["atX"].as<float>() : 0.f;
            e.archetype = eNode["archetype"] ? EnemyArchetypeRegistry::getId(eNode["archetype"].as<std::string>()) : 0;
            e.spawnType = eNode["spawnType"] ? Enemy::spawnTypeFromString(eNode["spawnType"].as<std::string>()) : EnemySpawnType::Simple;
            e.count = eNode["count"] ? eNode["count"].as<int>() : 1;
            e.spacing = eNode["spacing"] ? eNode["spacing"].as<float>() : 0.f;
            e.formation = eNode["formation"] ? eNode["formation"].as<std::string>() : "";
            e.y = eNode["y"] ? eNode["y"].as<float>() : -1.f;

            const auto &arche = EnemyArchetypeRegistry::get(e.archetype);
            if (eNode["velocity"])
            {
                e.velocity.x = eNode["velocity"]["x"] ? eNode["velocity"]["x"].as<float>() : arche.velocity.x;
                e.velocity.y = eNode["velocity"]["y"] ? eNode["velocity"]["y"].as<float>() : arche.velocity.y;
            }
            else
            {
                e.velocity = arche.velocity;
            }

            lvl.enemies.push_back(e);
        }
    }

    std::sort(lvl.segments.begin(), lvl.segments.end(),
              [](const auto &a, const auto &b)
              { return a.atX < b.atX; });
    std::sort(lvl.enemies.begin(), lvl.enemies.end(),
              [](const auto &a, const auto &b)
              { return a.atX < b.atX; });

    levels[lvl.id] = lvl;
}

const LevelDesc *LevelRegistry::get(const std::string &id)
{
    auto it = levels.find(id);
    return it != levels.end() ? &it->second : nullptr;
}

const LevelDesc *LevelRegistry::current()
{
    return currentLevel;
}

void LevelRegistry::setCurrent(const std::string &id)
{
    currentLevel = get(id);
    if (!currentLevel)
        std::cerr << "LevelRegistry: level not found: " << id << "\n";
}
