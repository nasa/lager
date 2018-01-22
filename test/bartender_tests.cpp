#include <memory>

#include <gtest/gtest.h>

#include "bartender.h"
#include "lager_utils.h"

class BartenderTests : public ::testing::Test
{
protected:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {

    }
};

TEST_F(BartenderTests, DoesItWork)
{
    Bartender b;
    b.init(12345);
    b.start();

    lager_utils::sleep(1000);

    b.stop();
}
