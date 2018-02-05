#include <zmq.hpp>

#include "mug.h"
#include "lager_utils.h"

int main(int argc, char* argv[])
{
    std::cout << "zmq version: " << lager_utils::getZmqVersion() << std::endl;

    Mug m;
    m.init("localhost", 12345);
    m.start();

    lager_utils::sleepMillis(60000);

    std::vector<AbstractDataRefItem*> dataRefItems;

    m.stop();

    return 0;
}
