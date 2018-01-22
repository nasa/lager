#include <memory>

#include <gtest/gtest.h>

#include "mug.h"
#include "lager_utils.h"

class MugTests : public ::testing::Test
{
protected:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {

    }
};

TEST_F(MugTests, DoesItWork)
{
    Mug m;
    m.init("localhost", 12345);
    m.start();

    lager_utils::sleep(1000);

    m.stop();
}
