// EntityManager.hpp
#pragma once
#include "Entity.hpp"
#include <vector>
#include <mutex>

class EntityManager
{
public:
    std::vector<Entity> entities;
    std::mutex mtx;

    void add(const Entity &e)
    {
        std::lock_guard<std::mutex> lock(mtx);
        entities.push_back(e);
    }

    void update(float dt)
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto &e : entities)
            if (e.alive)
                e.update(dt);
    }

    void draw(sf::RenderWindow &w)
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto &e : entities)
            if (e.alive)
                e.draw(w);
    }

    void removeDead()
    {
        std::lock_guard<std::mutex> lock(mtx);
        entities.erase(std::remove_if(entities.begin(), entities.end(),
                                      [](const Entity &e)
                                      { return !e.alive; }),
                       entities.end());
    }
};
