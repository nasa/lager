#include <memory>

#include <gtest/gtest.h>

#include "bartender.h"
#include "mug.h"
#include "tap.h"
#include "lager_utils.h"

class EndToEndTests : public ::testing::Test
{
protected:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {

    }
};

TEST_F(EndToEndTests, SubscriberUpdates)
{
    uint32_t item1 = 0;

    Bartender b;
    b.init(12345);

    Mug m;
    m.init("localhost", 12345);

    b.start();
    m.start();

    lager_utils::sleepMillis(2000);

    Tap t;
    t.init("localhost", 12345);
    t.addItem(new DataRefItem<uint32_t>("item1", &item1));
    t.start("/test");

    for (unsigned int i = 0; i < 5; ++i)
    {
        item1++;
        t.log();
        lager_utils::sleepMillis(100);
    }

    m.stop();
    t.stop();
    b.stop();
}

// TODO fail if this hangs somehow
TEST_F(EndToEndTests, DoesItWork)
{
    uint32_t item1 = 0;
    uint16_t item2 = 0;
    uint8_t item3 = 0;
    uint64_t item4 = 0;

    Bartender b;
    b.init(12345);

    Mug m;
    m.init("localhost", 12345);

    Tap t;
    t.init("localhost", 12345);

    b.start();
    m.start();

    lager_utils::sleepMillis(1000);

    t.addItem(new DataRefItem<uint32_t>("item1", &item1));
    t.addItem(new DataRefItem<uint16_t>("item2", &item2));
    t.addItem(new DataRefItem<uint8_t>("item3", &item3));
    t.addItem(new DataRefItem<uint64_t>("item4", &item4));
    t.start("/test");

    for (unsigned int i = 0; i < 5; ++i)
    {
        item1++;
        item2++;
        item3++;
        item4++;
        t.log();
        lager_utils::sleepMillis(1000);
    }

    m.stop();
    t.stop();
    b.stop();
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
