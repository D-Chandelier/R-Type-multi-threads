#pragma once
#include <iostream>

struct RemotePlayer
{
    int id;
    float x;
    float y;
    double lastUpdateTime; // serveur
    double serverX, serverY;
    float lerpFactor = 0.3f; // fluidité du mouvement, 0 = pas de mouvement, 1 = téléport
    ENetPeer *peer = nullptr;
};