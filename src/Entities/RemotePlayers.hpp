#pragma once
#include <iostream>

struct RemotePlayer
{
    int id;
    float x;
    float y;
    ENetPeer *peer = nullptr;
};