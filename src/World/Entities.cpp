#include "Entities.hpp"

std::unordered_map<std::string, EnemyArchetypeId>
    EnemyArchetypeRegistry::nameToId;
std::unordered_map<EnemyArchetypeId, EnemyArchetype>
    EnemyArchetypeRegistry::archetypes;

void EnemyArchetypeRegistry::loadFromFile(const std::string &path)
{
    YAML::Node root = YAML::LoadFile(path);
    const auto &enemies = root["enemies"];

    EnemyArchetypeId nextId = 1;

    for (auto it = enemies.begin(); it != enemies.end(); ++it)
    {
        EnemyArchetype a;
        a.id = nextId;
        a.name = it->first.as<std::string>();

        const auto &n = it->second;

        if (n["velocity"])
        {
            a.velocity.x = n["velocity"]["x"] ? n["velocity"]["x"].as<float>() : 0.f;
            a.velocity.y = n["velocity"]["y"] ? n["velocity"]["y"].as<float>() : 0.f;
        }
        else
        {
            a.velocity = {0.f, 0.f};
        }
        if (n["size"])
        {
            a.size.x = n["size"]["x"] ? n["size"]["x"].as<float>() : 48.f;
            a.size.y = n["size"]["y"] ? n["size"]["y"].as<float>() : 48.f;
        }
        else
        {
            a.size = {48.f, 48.F};
        }
        a.pv = n["pv"] ? n["pv"].as<float>() : 1.f;
        a.points = n["points"] ? n["points"].as<float>() : 0.f;
        a.fireRate = n["fire_rate"] ? n["fire_rate"].as<float>() : 0.f;
        a.texture = n["texture"] ? n["texture"].as<std::string>() : "";

        nameToId[a.name] = nextId;
        archetypes[nextId] = a;
        nextId++;
    }
}

EnemyArchetypeId EnemyArchetypeRegistry::getId(const std::string &name)
{
    return nameToId.at(name);
}

const EnemyArchetype &EnemyArchetypeRegistry::get(EnemyArchetypeId id)
{
    return archetypes.at(id);
}
