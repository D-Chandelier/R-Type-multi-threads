#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "Tile.hpp"
#include "Segments.hpp"
#include "../Entities/Enemies.hpp"
#include <yaml-cpp/yaml.h>

struct LevelSegmentDesc;
struct EnemySpawnDesc;

struct LevelDesc
{
    std::string id;
    float scrollSpeed = 180.f;
    uint32_t seed = 0;
    sf::Vector2f tileSize = {64.f, 64.f};
    float segmentWidth = 1152.f;

    std::vector<LevelSegmentDesc> segments;
    std::vector<EnemySpawnDesc> enemies;
};

struct LevelRuntime
{
    const LevelDesc *desc = nullptr;

    size_t nextSegmentIndex = 0;
    size_t nextEnemyIndex = 0;

    float scrollSpeed() const { return desc ? desc->scrollSpeed : 0.f; }
    const std::vector<LevelSegmentDesc> &segments() const { return desc->segments; }
    const std::vector<EnemySpawnDesc> &enemies() const { return desc->enemies; }
};

class LevelRegistry
{
public:
    static void loadFromFile(const std::string &path);

    static const LevelDesc *get(const std::string &id);

    static const LevelDesc *current();
    static void setCurrent(const std::string &id);

private:
    static std::unordered_map<std::string, LevelDesc> levels;
    static const LevelDesc *currentLevel;
};
