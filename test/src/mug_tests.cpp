#include <memory>

#include <gtest/gtest.h>

#include "lager/mug.h"
#include "lager/lager_utils.h"

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

TEST_F(MugTests, BadPortNumber)
{
    Mug m;
    EXPECT_FALSE(m.init("localhost", -50));
    EXPECT_FALSE(m.init("localhost", 65535));
}

TEST_F(MugTests, DoesItWork)
{
    Mug m;
    m.init("localhost", 12345);
    m.start();

    lager_utils::sleepMillis(100);

    m.stop();
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
