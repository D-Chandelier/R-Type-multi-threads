#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include "BlockVisual.hpp"

using TileId = uint16_t;

struct TileEntry
{
    TileId id;
    std::string name;
    sf::Texture texture;
};

class ServerTileRegistry
{
public:
    static void loadFromFile(const std::string &path);
    static TileId getId(const std::string &name);

private:
    static std::unordered_map<std::string, TileId> nameToId;
};

class ClientTileRegistry
{
public:
    static void loadFromFile(const std::string &path);

    static sf::Texture &getTexture(TileId id);
    static TileId getId(const std::string &name);
    static sf::IntRect getTextureRect(BlockVisual visual, const sf::Texture &tex);

private:
    static std::unordered_map<std::string, TileId> nameToId;
    static std::unordered_map<TileId, std::shared_ptr<sf::Texture>> textures;
    static std::shared_ptr<sf::Texture> defaultTexture;
};