#include <fstream>
#include <stdint.h>
#include <streambuf>
#include <string>

#include <zmq.hpp>

#include "tap.h"
#include "lager_utils.h"
#include "data_ref_item.h"

int main(int argc, char* argv[])
{
    std::cout << "zmq version: " << lager_utils::getZmqVersion() << std::endl;

    Tap t;
    t.init("localhost", 12345);

    uint32_t int1 = 0;
    int32_t int2 = -500;
    double double1 = 0.001;

    t.addItem(new DataRefItem<uint32_t>("int1", &int1));
    t.addItem(new DataRefItem<int32_t>("int2", &int2));
    t.addItem(new DataRefItem<double>("double1", &double1));

    t.start("/sample_format");

    for (unsigned int i = 0; i < 1000; ++i)
    {
        t.log();

        lager_utils::sleepMillis(500);

        int1 += 10;
        int2 += 100;
        double1 += 0.001;
    }

    t.stop();

    return 0;
}
