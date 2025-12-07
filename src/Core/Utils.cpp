#include "Utils.hpp"

std::string Utils::getLocalIpAddress()
{
    auto optIp = sf::IpAddress::getLocalAddress();
    if (!optIp.has_value())
        return "0.0.0.0"; // ou "" selon ton besoin

    return optIp->toString();
}
