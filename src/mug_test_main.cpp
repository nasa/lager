#include <zmq.hpp>

#include "lager/mug.h"
#include "lager/lager_utils.h"

// TODO this test application will be removed
int main(int argc, char* argv[])
{
    std::cout << "zmq version: " << lager_utils::getZmqVersion() << std::endl;

    Mug m;
    m.init("localhost", 12345, 100);
    m.start();

    do
    {
        std::cout << "Press enter to end" << std::endl;
    }
    while (std::cin.get() != '\n');

    m.stop();

    return 0;
}
