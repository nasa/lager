#include "lager/bartender.h"
#include "lager/lager_utils.h"

// TODO this test application will be removed
int main(int argc, char* argv[])
{
    std::cout << "zmq version: " << lager_utils::getZmqVersion() << std::endl;

    Bartender b;
    b.init(12345);
    b.start();

    do
    {
        std::cout << "Press enter to end" << std::endl;
    }
    while (std::cin.get() != '\n');

    b.stop();

    return 0;
}
