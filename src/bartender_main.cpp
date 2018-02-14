#include "lager/bartender.h"
#include "lager/lager_utils.h"

// TODO this test application will be removed
int main(int argc, char* argv[])
{
    std::cout << "zmq version: " << lager_utils::getZmqVersion() << std::endl;

    Bartender b;
    b.init(12345);
    b.start();

    lager_utils::sleepMillis(60000);

    b.stop();

    return 0;
}
