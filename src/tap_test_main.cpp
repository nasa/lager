#include <fstream>
#include <streambuf>
#include <string>

#include <zmq.hpp>

#include "tap.h"
#include "lager_utils.h"

int main(int argc, char* argv[])
{
    std::cout << "zmq version: " << lager_utils::getZmqVersion() << std::endl;

    Tap t;
    t.init("localhost", 12345);
    t.start("/sample_format", "sample_format.xml");

    lager_utils::sleep(1000);

    for (unsigned int i = 0; i < 5; ++i)
    {
        t.log(i);
        lager_utils::sleep(500);
    }

    t.stop();

    return 0;
}
