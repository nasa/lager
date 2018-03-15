#include "lager/tap.h"
#include "lager/bartender.h"
#include "lager/mug.h"

int main(int argc, char* argv[])
{
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

    t.addItem(new DataRefItem<uint32_t>("uint1", &uint1));
    t.addItem(new DataRefItem<int32_t>("int1", &int1));
    t.addItem(new DataRefItem<double>("double1", &double1));
    t.addItem(new DataRefItem<uint16_t>("ushort1", &ushort1));
    t.addItem(new DataRefItem<int16_t>("short1", &short1));
    t.addItem(new DataRefItem<uint8_t>("ubyte1", &ubyte1));
    t.addItem(new DataRefItem<int8_t>("byte1", &byte1));
    t.addItem(new DataRefItem<float>("float1", &float1));

    t.start("/sample_format");

    for (unsigned int i = 0; i < 1000; ++i)
    {
        t.log();

        lager_utils::sleepMillis(5);

        ubyte1 += 1;
        byte1 += 10;
        ushort1 += 10;
        short1 += 100;
        uint1 += 10;
        int1 += 100;
        double1 += 0.001;
        float1 += 0.010f;
    }

    t.stop();
    m.stop();
    b.stop();
}
