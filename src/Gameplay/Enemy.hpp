#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <string>
#include <iostream>
#include <cmath>

enum class EnemyBehavior {
    Linear,         // mouvement droit classique
    SineWave,       // vague sinus
    Dive,           // piqué vers le joueur
    ZigZag,         // zig zag
    HoverShoot,     // avance puis stationne et tire
    BigCurve        // grande courbe façon R-Type
};


enum class EnemyType { 
    enemyBlack1,
    enemyBlack2,
    enemyBlack3,
    enemyBlack4,
    enemyBlack5,
    enemyBlack6,
    enemyBlue1,
    enemyBlue2,
    enemyBlue3,
    enemyBlue4,
    enemyBlue5,
    enemyBlue6,
    enemyGreen1,
    enemyGreen2,
    enemyGreen3,
    enemyGreen4,
    enemyGreen5,
    enemyGreen6,
    enemyRed1,
    enemyRed2,
    enemyRed3,
    enemyRed4,
    enemyRed5,
    enemyRed6
};

struct Enemy {
    EnemyType type;
    sf::Vector2f pos;
    sf::Vector2f size;        // taille logique
    sf::Vector2f displaySize = size; // taille pour affichage et hitbox

    EnemyBehavior behavior = EnemyBehavior::Linear;
    float behaviorTime = 0.f;   // timer interne pour animé le pattern

    float speed;
    int hp;

    sf::Texture* texture = nullptr;
    int frameCount = 1;
    float animSpeed = 0.1f; // secondes par frame
    int currentFrame = 0;
    float animTimer = 0.f;
    sf::Angle rotation = sf::degrees(0);

    // Shield / effet de hit
    bool shieldActive = false;
    float shieldTimer = 0.f;
    float shieldDuration = 0.5f; // durée du bouclier en secondes
    sf::Vector2f hitDir = {1,0};

    // Gastion du tire
    sf::Texture enemyBulletTex;
    sf::Texture* bulletTexture = nullptr; 

    float shootCooldown = 0.f;     // temps avant prochain tir
    float shootRate = 2.f;        // cadence (modifiable par type)
    bool canShoot = true;          // certains ennemis ne tirent pas

    void setBulletTexture(sf::Texture* tex){bulletTexture = tex;};

    Enemy(EnemyType t, sf::Vector2f p, sf::Texture* tex)
        : type(t), pos(p), texture(tex)
    {
        switch (type) {
            case EnemyType::enemyBlack1: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90); speed=140.f; hp=1; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyBlack2: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90); speed=130.f; hp=2; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyBlack3: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90); speed=120.f; hp=3; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyBlack4: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90); speed=110.f; hp=5; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyBlack5: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90); speed=100.f; hp=8; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyBlack6: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90); speed=100.f; hp=10; frameCount=1; animSpeed=0.15f; break;

            case EnemyType::enemyBlue1: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=140.f; hp=1; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyBlue2: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=130.f; hp=2; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyBlue3: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=120.f; hp=3; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyBlue4: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=110.f; hp=5; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyBlue5: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=100.f; hp=8; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyBlue6: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=100.f; hp=10; frameCount=1; animSpeed=0.15f; break;
        
            case EnemyType::enemyGreen1: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=140.f; hp=1; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyGreen2: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=130.f; hp=2; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyGreen3: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=120.f; hp=3; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyGreen4: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=110.f; hp=5; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyGreen5: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=100.f; hp=8; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyGreen6: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=100.f; hp=10; frameCount=1; animSpeed=0.15f; break;
        
            case EnemyType::enemyRed1: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=140.f; hp=1; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyRed2: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=130.f; hp=2; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyRed3: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=120.f; hp=3; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyRed4: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=110.f; hp=5; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyRed5: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=100.f; hp=8; frameCount=1; animSpeed=0.15f; break;
            case EnemyType::enemyRed6: size = {93.f,84.f}; displaySize= {31.f,28.f}; rotation=sf::degrees(90.f); speed=100.f; hp=10; frameCount=1; animSpeed=0.15f; break;
        
        }
    }

    // get AABB (pos en centre)
    sf::FloatRect getBounds() const {
        return sf::FloatRect(
            {pos.x - displaySize.x * 0.5f, pos.y - displaySize.y * 0.5f},
            {displaySize.x, displaySize.y});
    }

    sf::FloatRect getTransformedBounds() const {
        if (!texture) return getBounds();
        sf::Sprite tmp(*texture);
        tmp.setTextureRect(getFrameRect());
        tmp.setOrigin({size.x * 0.5f, size.y * 0.5f});
        tmp.setPosition(pos);
        tmp.setRotation(rotation);
        tmp.setScale({displaySize.x / size.x, displaySize.y / size.y});
        return tmp.getGlobalBounds();
    }

    float getCollisionRadius() const {
        return std::max(displaySize.x, displaySize.y) * 0.5f * 0.85f;
    }


    bool isDead() const { return hp <= 0; }

    void updateAnimation(float dt) {
        if (frameCount <= 1) return;
        animTimer += dt;
        if (animTimer >= animSpeed) {
            currentFrame = (currentFrame + 1) % frameCount;
            animTimer = 0.f;
        }
    }

    sf::IntRect getFrameRect() const {
        return sf::IntRect(
            {currentFrame * static_cast<int>(size.x), 0},
            {static_cast<int>(size.x), static_cast<int>(size.y)});
    }

    // update général : animation + shield timer
    void update(float dt) {
        updateAnimation(dt);
        if (shieldActive) {
            shieldTimer -= dt;
            if (shieldTimer <= 0.f) {
                shieldActive = false;
                shieldTimer = 0.f;
            }
        }
    }

    // utility : activer le shield (appelé lors d'une collision si hp>0)
    void activateShield(float duration = -1.f) {
        shieldActive = true;
        shieldTimer = (duration > 0.f) ? duration : shieldDuration;
    }

    void updateMovement(float dt, sf::Vector2f playerPos) {
        behaviorTime += dt;

        switch (behavior) {

            case EnemyBehavior::Linear:
                pos.x -= speed * dt;
                break;

            case EnemyBehavior::SineWave:
                pos.x -= speed * dt;
                pos.y += std::sin(behaviorTime * 4.f) * 40.f * dt;
                break;

            case EnemyBehavior::ZigZag:
                pos.x -= speed * dt;
                pos.y += ((int(behaviorTime * 2) % 2 == 0) ? 1 : -1) * 120.f * dt;
                break;

            case EnemyBehavior::Dive:
                pos.x -= speed * dt;
                // se dirige légèrement vers le joueur
                pos.y += (playerPos.y - pos.y) * 0.5f * dt;
                break;

            case EnemyBehavior::HoverShoot:
                if (behaviorTime < 2.5f)
                    pos.x -= speed * dt;
                else
                    pos.x -= speed * 0.2f * dt;  // ralentit

                // petit tremblement stationnaire
                pos.y += std::sin(behaviorTime * 6.f) * 10.f * dt;
                if (behaviorTime > 5.f)
                    behaviorTime = 0.f;
                break;

            case EnemyBehavior::BigCurve:
                pos.x -= speed * dt;
                pos.y += std::sin(behaviorTime * 2.f) * 80.f * dt;
                break;
        }
    }    
};
// Comportement	Mouvement	        Style de tir
// Linear   	ligne droite	    tir simple
// SineWave	    oscillation	        tir périodique
// ZigZag	    gauche/droite	    tir simple
// Dive	        vise le joueur	    tir tracking
// HoverShoot	avance + stationne	tir lent puis rafales
// BigCurve	    grosses vagues	    tir en éventail