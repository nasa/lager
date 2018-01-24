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

    uint32_t int1 = 0;
    uint32_t int2 = 0;

    t.addItem(DataRefItem("int1", int1));
    t.addItem(DataRefItem("int2", int2));

    t.start("/sample_format", "sample_format.xml");

    lager_utils::sleep(1000);


    for (unsigned int i = 0; i < 1000; ++i)
    {
        t.log();

        lager_utils::sleep(500);

        int1 += 10;
        int2 += 100;
    }

    t.stop();

    return 0;
}
