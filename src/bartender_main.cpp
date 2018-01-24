#include "bartender.h"
#include "lager_utils.h"

int main(int argc, char* argv[])
{
    std::cout << "zmq version: " << lager_utils::getZmqVersion() << std::endl;

    Bartender b;
    b.init(12345);
    b.start();

    lager_utils::sleep(20000);

    b.stop();

    return 0;
}
