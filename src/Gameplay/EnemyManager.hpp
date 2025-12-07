#pragma once
#include "Enemy.hpp"
#include <vector>
#include <SFML/Graphics.hpp>
#include "BulletManager.hpp"

class EnemyManager {
public:
    EnemyManager();
    void spawn(EnemyType type, float y);
    void spawnRandom(float y);
    void spawnMultiple(int count);
    void update(float dt, sf::Vector2f playerPos, BulletManager& bulletMgr);
    //void drawBouclier(sf::RenderWindow window);
    void draw(sf::RenderWindow& window);

    std::vector<Enemy>& accessEnemies() { return enemies; }

private:
    std::vector<Enemy> enemies;

    sf::Texture enemyBlack1;
    sf::Texture enemyBlack2;
    sf::Texture enemyBlack3;
    sf::Texture enemyBlack4;
    sf::Texture enemyBlack5;
    sf::Texture enemyBlack6;
    sf::Texture enemyBlue1;
    sf::Texture enemyBlue2;
    sf::Texture enemyBlue3;
    sf::Texture enemyBlue4;
    sf::Texture enemyBlue5;
    sf::Texture enemyBlue6;
    sf::Texture enemyGreen1;
    sf::Texture enemyGreen2;
    sf::Texture enemyGreen3;
    sf::Texture enemyGreen4;
    sf::Texture enemyGreen5;
    sf::Texture enemyGreen6;
    sf::Texture enemyRed1;
    sf::Texture enemyRed2;
    sf::Texture enemyRed3;
    sf::Texture enemyRed4;
    sf::Texture enemyRed5;
    sf::Texture enemyRed6;

    sf::Texture* getTexture(EnemyType type);
};
