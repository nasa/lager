#include <sstream>

#include "lager/tap.h"
#include "lager/bartender.h"
#include "lager/mug.h"

void usage(char* progName)
{
    std::cout << progName << " [options] ARRAY_SIZE" << std::endl <<
              "Options:" << std::endl <<
              "-h | --help        Print this help" << std::endl <<
              "ARRAY_SIZE         Size of uint32_t array to add to the tap (defaults to 10)" << std::endl;
}

int main(int argc, char* argv[])
{
    int arraySize = 10;

    if (argc == 2)
    {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        {
            usage(argv[0]);
            return 0;
        }

        std::istringstream ss(argv[1]);

        if (!(ss >> arraySize))
        {
            std::cerr << "Invalid ARRAY_SIZE: " << argv[1] << std::endl;
            return 1;
        }
    }

    if (argc > 2)
    {
        usage(argv[0]);
        return 0;
    }

    Tap t;
    Mug m;
    Bartender b;

    b.init(12345);
    t.init("localhost", 12345, 100);
    m.init("localhost", 12345, 100);
    m.start();
    b.start();

    uint8_t ubyte1 = 0;
    int8_t byte1 = -100;
    uint16_t ushort1 = 0;
    int16_t short1 = -1000;
    uint32_t uint1 = 0;
    int32_t int1 = -1000;
    double double1 = 0.001;
    float float1 = 0.001f;
    std::vector<uint32_t> array;
    array.resize(arraySize);

    t.addItem(new DataRefItem<uint32_t>("uint1", &uint1));
    t.addItem(new DataRefItem<int32_t>("int1", &int1));
    t.addItem(new DataRefItem<double>("double1", &double1));
    t.addItem(new DataRefItem<uint16_t>("ushort1", &ushort1));
    t.addItem(new DataRefItem<int16_t>("short1", &short1));
    t.addItem(new DataRefItem<uint8_t>("ubyte1", &ubyte1));
    t.addItem(new DataRefItem<int8_t>("byte1", &byte1));
    t.addItem(new DataRefItem<float>("float1", &float1));

    std::stringstream ss;

    for (unsigned int i = 0; i < arraySize; ++i)
    {
        ss.str(std::string());
        ss << "array" << i;
        t.addItem(new DataRefItem<uint32_t>(ss.str(), &array[i]));
    }

    t.start("/sample_format");

    for (unsigned int i = 0; i < 1000; ++i)
    {
        t.log();

        ubyte1 += 1;
        byte1 += 10;
        ushort1 += 10;
        short1 += 100;
        uint1 += 10;
        int1 += 100;
        double1 += 0.001;
        float1 += 0.010f;

        for (unsigned int i = 0; i < arraySize; ++i)
        {
            array[i] += 1;
        }

        lager_utils::sleepMillis(1);
    }

    t.stop();
    m.stop();
    b.stop();
}
