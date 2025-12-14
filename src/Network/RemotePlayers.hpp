#pragma once
#include <iostream>

struct RemotePlayer
{
    uint32_t id;
    float x, y;
    ENetPeer* peer; // pour identifier le joueur lors d'une déconnexion
};