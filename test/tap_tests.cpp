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

TEST_F(TapTests, DoesItWork)
{
    Tap t;
    t.init("localhost", 12345);
    t.start();

    lager_utils::sleep(1000);

    for (unsigned int i = 0; i < 5; ++i)
    {
        t.log(i);
        lager_utils::sleep(500);
    }

    t.stop();
}
