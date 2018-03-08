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

    lager_utils::sleepMillis(10000);

    m.stop();

    return 0;
}
