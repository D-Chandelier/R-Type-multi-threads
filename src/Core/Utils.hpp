#include <SFML/Network.hpp>
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
