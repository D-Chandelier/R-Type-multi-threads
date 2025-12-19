#pragma once
#include <SFML/Network.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/String.hpp>
#include <string>

// std::string getLocalIpAddress()
// {
//     auto optIp = sf::IpAddress::getLocalAddress();
//     if (!optIp.has_value())
//         return "0.0.0.0"; // ou "" selon ton besoin

//     return optIp->toString();
// }

static double localTimeNow()
{
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}
static std::string keyToString(sf::Keyboard::Scancode k)
{
    return sf::Keyboard::getDescription(k).toAnsiString(); // SFML >= 2.6
}
