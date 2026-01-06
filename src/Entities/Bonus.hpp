#pragma once
#include "../Core/Config.hpp"
#include "RemotePlayers.hpp"
#include <SFML/Graphics.hpp>

class Client;
class Server;

static constexpr int BONUS_TYPE_COUNT = 5;

constexpr float BONUS_HEIGHT = 48.f;
constexpr float BONUS_WIDTH = 48.f;
constexpr float BONUS_SPEED = 50.f;
constexpr float BONUS_SCROLL_SPEED = -60.f; // vers la gauche
constexpr float BONUS_ORBIT_SPEED = 0.6f;   // radians / seconde
constexpr float BONUS_SPIN_SPEED = 90.f;    // degrés / seconde

constexpr sf::Color BONUS_COLOR = sf::Color::Cyan;

enum class BonusType
{
    RocketX3,
    HealthX1,
    Shield,
    FireRateUp,
    ScoreBoost
};

struct Bonus
{
    uint32_t id;
    BonusType type;

    sf::Vector2f spawnPos = {0.f, 0.f}; // point d’origine
    sf::Vector2f velocity = {0.F, 0.f}; // vitesse horizontale

    float time = 0.f;         // temps depuis le spawn
    float amplitude = 0.f;    // hauteur du cercle (ex: 300px)
    float angularSpeed = 0.f; // rad/sec
    float phase = 0.f;        // déphasage

    sf::Vector2f position = {0.f, 0.f}; // position calculée (résultat)

    bool active = true;

public:
    static Bonus &Get()
    {
        static Bonus instance;
        return instance;
    }
    static void updateBonusesClient(Client &client, float dt);
    static void buildBonusQuad(const Bonus &b, sf::VertexArray &va);
    static void drawBonuses(sf::RenderWindow &w);

    static void updateBonusesServer(Server &server, float dt);
    static bool checkCollision(const Bonus &b, const RemotePlayer &p, float worldX);

    inline static float radToDeg(float r) { return r * 180.f / 3.14159265f; }
    inline static float bulletAngle(const sf::Vector2f &v) { return radToDeg(std::atan2(v.y, v.x)); }

    sf::VertexArray RocketX3VA, HealthX1VA, ShieldVA, FireRateUpVA, ScoreBoostVA;
};

struct BonusStats
{
    int turretsDestroyed = 0;
};
