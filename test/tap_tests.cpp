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
    Tap t;
    t.init("localhost", 12345);
    t.start("test_format", "good_format.xml");

    lager_utils::sleep(1000);

    for (unsigned int i = 0; i < 5; ++i)
    {
        t.log();
        lager_utils::sleep(500);
    }

    t.stop();
}

TEST_F(TapTests, DoesItWorkString)
{
    Tap t;
    t.init("localhost", 12345);
    t.start("test_format", "<?xml version=\"1.0\" encoding=\"UTF-8\"?><format version=\"BEERR01\">"
            "<item name=\"column1\" type=\"string\" size=\"255\" offset=\"0\"/>"
            "<item name=\"column2\" type=\"integer\" size=\"4\" offset=\"255\"/></format>", false);

    lager_utils::sleep(1000);

    for (unsigned int i = 0; i < 5; ++i)
    {
        t.log();
        lager_utils::sleep(500);
    }

    t.stop();
}
