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

TEST_F(TapTests, DoesItWorkFile)
{
    uint32_t i = 0;
    Tap t;
    t.init("localhost", 12345);
    t.start("test_format");
    t.addItem(new DataRefItem<uint32_t>("item1", i));

    lager_utils::sleep(1000);

    for (unsigned int i = 0; i < 5; ++i)
    {
        t.log();
        lager_utils::sleep(500);
    }

    t.stop();
}
