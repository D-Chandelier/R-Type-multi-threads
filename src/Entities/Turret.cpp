#include "Turret.hpp"
#include "../World/Terrain.hpp"

// Turret::Turret(sf::Vector2f pos)
//     : position(pos) {}

// void Turret::update(float dt) //, Terrain& /*terrain*/)
// {
//     if (!active)
//         return;

//     shootCooldown -= dt;

//     if (shootCooldown <= 0.f)
//     {
//         // shootFromTurret(*this);
//         shootCooldown = 1.f;
//     }
// }

// inline void Turret::update(Terrain &terrain, float dt)
// {
//     for (auto &seg : terrain.segments)
//     {
//         for (auto &turret : seg.turrets)
//         {
//             if (!turret.active)
//                 continue;

//             turret.shootCooldown -= dt;
//             if (turret.shootCooldown <= 0.f)
//             {
//                 // shootFromTurret(turret);
//                 turret.shootCooldown = 1.f; // 1 seconde entre tirs par exemple
//             }
//         }
//     }
// }