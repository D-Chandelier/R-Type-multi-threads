#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include <yaml-cpp/yaml.h>
#include <unordered_map>

using EnemyArchetypeId = uint16_t;

struct EnemyArchetype
{
    EnemyArchetypeId id;
    std::string name;

    std::string texturePath;
    sf::Vector2f velocity;
    float pv = 1.f;
    float points = 0.f;
    float fireRate = 0.f;
    sf::Vector2f size = {48.f, 48.F};

    std::string texture;
};
class EnemyArchetypeRegistry
{
public:
    static void loadFromFile(const std::string &path);

    static EnemyArchetypeId getId(const std::string &name);
    static const EnemyArchetype &get(EnemyArchetypeId id);

    static std::unordered_map<std::string, EnemyArchetypeId> nameToId;
    static std::unordered_map<EnemyArchetypeId, EnemyArchetype> archetypes;
};
