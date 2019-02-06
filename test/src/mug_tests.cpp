#include <memory>

#include <gtest/gtest.h>

#include "lager/mug.h"
#include "lager/lager_utils.h"

TEST(MugTests, BadPortNumber)
{
    Mug m;
    EXPECT_FALSE(m.init("localhost", -50, 100));
    EXPECT_FALSE(m.init("localhost", 65535, 100));
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
