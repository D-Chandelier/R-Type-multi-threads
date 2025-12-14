#include <SFML/Graphics.hpp>
#include "Core/Game.hpp"
#include <cmath>

int main()
{
    if (enet_initialize() != 0) {
        std::cerr << "ENet init failed\n";
        return 1;
    }

    {
        Game game;
        game.run();
    }

    enet_deinitialize();
    return 0;
}
