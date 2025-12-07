#include "EnemyManager.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>

EnemyManager::EnemyManager() {
    if (!enemyBlack1.loadFromFile("assets/enemyBlack1.png"))
        std::cerr << "Erreur chargement enemyBlack1.png\n";
    if (!enemyBlack2.loadFromFile("assets/enemyBlack2.png"))
        std::cerr << "Erreur chargement enemyBlack2.png\n";
    if (!enemyBlack3.loadFromFile("assets/enemyBlack3.png"))
        std::cerr << "Erreur chargement enemyBlack3.png\n";
    if (!enemyBlack4.loadFromFile("assets/enemyBlack4.png"))
        std::cerr << "Erreur chargement enemyBlack4.png\n";
    if (!enemyBlack5.loadFromFile("assets/enemyBlack5.png"))
        std::cerr << "Erreur chargement enemyBlack5.png\n";
    if (!enemyBlack6.loadFromFile("assets/enemyBlack6.png"))
        std::cerr << "Erreur chargement enemyBlack6.png\n";
    
    if (!enemyBlue1.loadFromFile("assets/enemyBlue1.png"))
        std::cerr << "Erreur chargement enemyBlue1.png\n";
    if (!enemyBlue2.loadFromFile("assets/enemyBlue2.png"))
        std::cerr << "Erreur chargement enemyBlue2.png\n";
    if (!enemyBlue3.loadFromFile("assets/enemyBlue3.png"))
        std::cerr << "Erreur chargement enemyBlue3.png\n";
    if (!enemyBlue4.loadFromFile("assets/enemyBlue4.png"))
        std::cerr << "Erreur chargement enemyBlue4.png\n";
    if (!enemyBlue5.loadFromFile("assets/enemyBlue5.png"))
        std::cerr << "Erreur chargement enemyBlue5.png\n";   
    if (!enemyBlue6.loadFromFile("assets/enemyBlue6.png"))
        std::cerr << "Erreur chargement enemyBlue6.png\n";   
        
    if (!enemyGreen1.loadFromFile("assets/enemyGreen1.png"))
        std::cerr << "Erreur chargement enemyGreen1.png\n";
    if (!enemyGreen2.loadFromFile("assets/enemyGreen2.png"))
        std::cerr << "Erreur chargement enemyGreen2.png\n";
    if (!enemyGreen3.loadFromFile("assets/enemyGreen3.png"))
        std::cerr << "Erreur chargement enemyGreen3.png\n";
    if (!enemyGreen4.loadFromFile("assets/enemyGreen4.png"))
        std::cerr << "Erreur chargement enemyGreen4.png\n";
    if (!enemyGreen5.loadFromFile("assets/enemyGreen5.png"))
        std::cerr << "Erreur chargement enemyGreen5.png\n"; 
    if (!enemyGreen6.loadFromFile("assets/enemyGreen6.png"))
        std::cerr << "Erreur chargement enemyGreen6.png\n"; 
        
    if (!enemyRed1.loadFromFile("assets/enemyRed1.png"))
        std::cerr << "Erreur chargement enemyRed1.png\n";
    if (!enemyRed2.loadFromFile("assets/enemyRed2.png"))
        std::cerr << "Erreur chargement enemyRed2.png\n";
    if (!enemyRed3.loadFromFile("assets/enemyRed3.png"))
        std::cerr << "Erreur chargement enemyRed3.png\n";
    if (!enemyRed4.loadFromFile("assets/enemyRed4.png"))
        std::cerr << "Erreur chargement enemyRed4.png\n";
    if (!enemyRed5.loadFromFile("assets/enemyRed5.png"))
        std::cerr << "Erreur chargement enemyRed5.png\n"; 
    if (!enemyRed6.loadFromFile("assets/enemyRed6.png"))
        std::cerr << "Erreur chargement enemyRed6.png\n"; 
}

sf::Texture* EnemyManager::getTexture(EnemyType type) {
    switch(type) {
        case EnemyType::enemyBlack1: return &enemyBlack1;
        case EnemyType::enemyBlack2: return &enemyBlack2;
        case EnemyType::enemyBlack3: return &enemyBlack3;
        case EnemyType::enemyBlack4: return &enemyBlack4;
        case EnemyType::enemyBlack5: return &enemyBlack5;
        case EnemyType::enemyBlack6: return &enemyBlack6;
        
        case EnemyType::enemyBlue1: return &enemyBlue1;
        case EnemyType::enemyBlue2: return &enemyBlue2;
        case EnemyType::enemyBlue3: return &enemyBlue3;
        case EnemyType::enemyBlue4: return &enemyBlue4;
        case EnemyType::enemyBlue5: return &enemyBlue5;
        case EnemyType::enemyBlue6: return &enemyBlue6;

        case EnemyType::enemyGreen1: return &enemyGreen1;
        case EnemyType::enemyGreen2: return &enemyGreen2;
        case EnemyType::enemyGreen3: return &enemyGreen3;
        case EnemyType::enemyGreen4: return &enemyGreen4;
        case EnemyType::enemyGreen5: return &enemyGreen5;
        case EnemyType::enemyGreen6: return &enemyGreen6;

        case EnemyType::enemyRed1: return &enemyRed1;
        case EnemyType::enemyRed2: return &enemyRed2;
        case EnemyType::enemyRed3: return &enemyRed3;
        case EnemyType::enemyRed4: return &enemyRed4;
        case EnemyType::enemyRed5: return &enemyRed5;
        case EnemyType::enemyRed6: return &enemyRed6;        
    }
    return nullptr;
}

void EnemyManager::spawn(EnemyType type, float y) {
   
    Enemy& e = enemies.emplace_back(type, sf::Vector2f(1200.f, y), getTexture(type));

    if (!e.enemyBulletTex.loadFromFile("assets/laserBlue01.png"))
        std::cerr << "Erreur chargement laserBlue01.png\n";

    // Passe le pointeur à Player
    e.setBulletTexture(&e.enemyBulletTex);

    // exemple simple : par couleur
    switch (type) {
        case EnemyType::enemyBlack1:
        case EnemyType::enemyBlue1:
        case EnemyType::enemyGreen1:
        case EnemyType::enemyRed1:
            e.behavior = EnemyBehavior::Linear;
                break;
        case EnemyType::enemyBlack2:   
        case EnemyType::enemyBlue2:
        case EnemyType::enemyGreen2:
        case EnemyType::enemyRed2:        
            e.behavior = EnemyBehavior::SineWave;
            break;

        case EnemyType::enemyBlack3:
        case EnemyType::enemyBlue3:
        case EnemyType::enemyGreen3:
        case EnemyType::enemyRed3:
            e.behavior = EnemyBehavior::ZigZag;
            break;

        case EnemyType::enemyBlack4:
        case EnemyType::enemyBlue4:
        case EnemyType::enemyGreen4:
        case EnemyType::enemyRed4:
            e.behavior = EnemyBehavior::Dive;
            break;
        
        case EnemyType::enemyBlack5:
        case EnemyType::enemyBlue5:
        case EnemyType::enemyGreen5:
        case EnemyType::enemyRed5:
            e.behavior = EnemyBehavior::HoverShoot;
            break;

        default:
            e.behavior = EnemyBehavior::BigCurve;
    }
}

void EnemyManager::spawnRandom(float y) {
    
    int r = std::rand() % 24;
    spawn(static_cast<EnemyType>(r), y);
}

void EnemyManager::spawnMultiple(int count) {
    for (int i = 0; i < count; ++i) {
        float y = 50.f + std::rand() % 668;
        spawnRandom(y);
    }
}

void EnemyManager::update(float dt,sf::Vector2f playerPos , BulletManager& bulletMgr) {
    for (auto& e : enemies) {
        e.updateMovement(dt, playerPos);    
        e.update(dt);
        
        // Gérer le tir
        if (e.canShoot) {
            e.shootCooldown -= dt;
            if (e.shootCooldown <= 0.f) {
                // Exemple tir simple : vers la gauche
                sf::Vector2f bulletVel(-200.f, 0.f);

                bulletMgr.shoot(e.pos, bulletVel, e.bulletTexture, {12.f, 6.f}, BulletType::Enemy);
                
                e.shootCooldown = e.shootRate;
            }
        }
    }
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const Enemy& e){ return e.isDead() || e.pos.x < -100.f; }),
        enemies.end()
    );
}

static void drawShield(
    sf::RenderTarget& window,
    sf::Vector2f center,
    float radius,
    float angleDeg,
    float arcWidthDeg,
    sf::Color color,
    int points = 32
) {
    // nb de sommets = 1 (centre) + points + 1 (dernier point)
    sf::VertexArray fan(sf::PrimitiveType::TriangleFan, points + 2);

    float start = (angleDeg - arcWidthDeg * 0.5f) * (3.14159f / 180.f);
    float end   = (angleDeg + arcWidthDeg * 0.5f) * (3.14159f / 180.f);

    fan[0].position = center;
    fan[0].color = color;

    for (int i = 0; i <= points; i++) {
        float t = static_cast<float>(i) / points;
        float a = start + (end - start) * t;

        fan[i+1].position = {
            center.x + std::cos(a) * radius,
            center.y + std::sin(a) * radius
        };
        fan[i+1].color = color;
    }

    window.draw(fan);
}


void EnemyManager::draw(sf::RenderWindow& window) {
    for (auto& e : enemies) {
        if (!e.texture) continue;
        sf::Sprite sprite(*e.texture);
        sprite.setTextureRect(e.getFrameRect());
        sprite.setOrigin({e.size.x / 2, e.size.y / 2});
        sprite.setScale({e.displaySize.x / e.size.x, e.displaySize.y / e.size.y});
        sprite.setRotation(e.rotation);
        sprite.setPosition(e.pos);
        window.draw(sprite);

        
        if (e.shieldActive && e.hp > 0) {
            float scaleX = e.displaySize.x / e.size.x;
            float scaleY = e.displaySize.y / e.size.y;
            float maxScale = std::max(scaleX, scaleY);
            float baseSize = std::max(e.size.x, e.size.y);
            float radius = (baseSize * maxScale) * 0.6f + 4.f;

            // convertir hitDir -> angle (hitDir est déjà normalisé)
            float angle = std::atan2(e.hitDir.y, e.hitDir.x) * 180.f / 3.14159265f;

            std::uint8_t alpha = static_cast<std::uint8_t>(180.0f * (e.shieldTimer / e.shieldDuration));
            drawShield(window, e.pos, radius, angle, 80.f, sf::Color(100, 200, 255, alpha));
        }
    }
}
