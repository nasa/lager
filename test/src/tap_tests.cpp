#include <memory>

#include <gtest/gtest.h>

#include "tap.h"
#include "lager_utils.h"

class TapTests : public ::testing::Test
{
protected:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {

    }
};

TEST_F(TapTests, BadPortNumber)
{
    Tap t;
    EXPECT_FALSE(t.init("localhost", -50));
    EXPECT_FALSE(t.init("localhost", 65536));
}

TEST_F(TapTests, DoesItWork)
{
    uint32_t i = 0;
    Tap t;
    t.init("localhost", 12345);

    t.addItem(new DataRefItem<uint32_t>("item1", &i));
    t.start("/test");

    lager_utils::sleepMillis(1000);

    for (unsigned int i = 0; i < 5; ++i)
    {
        t.log();
        lager_utils::sleepMillis(500);
    }

    t.stop();
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
