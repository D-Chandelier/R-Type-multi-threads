#include "Tile.hpp"
#include <stdexcept>
#include <iostream>

std::unordered_map<std::string, TileId> ServerTileRegistry::nameToId;

void ServerTileRegistry::loadFromFile(const std::string &path)
{
    YAML::Node root = YAML::LoadFile(path);
    const auto &tiles = root["tiles"];

    TileId nextId = 1;

    for (auto it = tiles.begin(); it != tiles.end(); ++it)
    {
        std::string name = it->first.as<std::string>();
        nameToId[name] = nextId++;
    }
}

TileId ServerTileRegistry::getId(const std::string &name)
{
    auto it = nameToId.find(name);
    if (it == nameToId.end())
        return 1;
    return it->second;
}

std::unordered_map<std::string, TileId> ClientTileRegistry::nameToId;
std::unordered_map<TileId, std::shared_ptr<sf::Texture>> ClientTileRegistry::textures;
std::shared_ptr<sf::Texture> ClientTileRegistry::defaultTexture;

void ClientTileRegistry::loadFromFile(const std::string &path)
{
    YAML::Node root = YAML::LoadFile(path);
    const auto &tiles = root["tiles"];
    if (!tiles || !tiles.IsMap())
        throw std::runtime_error("Tiles.yaml: invalid 'tiles' node");

    TileId nextId = 1;

    for (auto it = tiles.begin(); it != tiles.end(); ++it)
    {
        const std::string name = it->first.as<std::string>();
        const std::string file = it->second["texture"].as<std::string>();

        auto tex = std::make_shared<sf::Texture>();
        if (!tex->loadFromFile(file))
            throw std::runtime_error("Failed to load tile texture: " + file);

        nameToId[name] = nextId;
        textures[nextId] = tex;

        if (name == "default")
            defaultTexture = tex;

        nextId++;
    }
    if (!defaultTexture)
        throw std::runtime_error("Tiles.yaml must define a 'default' tile");
}

TileId ClientTileRegistry::getId(const std::string &name)
{
    auto it = nameToId.find(name);
    if (it == nameToId.end())
        throw std::runtime_error("Unknown tile: " + name);
    return it->second;
}

sf::Texture &ClientTileRegistry::getTexture(TileId id)
{

    auto it = textures.find(id);
    if (it != textures.end())
        return *it->second;

    return *defaultTexture;
}

sf::IntRect ClientTileRegistry::getTextureRect(BlockVisual visual, const sf::Texture &tex)
{
    int w = tex.getSize().x / 3;
    int h = tex.getSize().y;

    switch (visual)
    {
    case BlockVisual::Left:
        return sf::IntRect({0, 0}, {w, h});
    case BlockVisual::Middle:
        return sf::IntRect({w, 0}, {w, h});
    case BlockVisual::Right:
        return sf::IntRect({2 * w, 0}, {w, h});
    default:
        return sf::IntRect({0, 0}, {w, h});
    }
}