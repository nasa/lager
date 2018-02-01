#include <gtest/gtest.h>

#include "lager_defines.h"
#include "lager_utils.h"

class LagerUtilTests : public ::testing::Test
{};

TEST_F(LagerUtilTests, UuidLength)
{
#ifndef _WIN32
    ASSERT_EQ(lager_utils::getUuid().size(), UUID_SIZE_BYTES);
#endif
}

TEST_F(LagerUtilTests, LocalUri)
{
    ASSERT_STREQ(lager_utils::getLocalUri(12345).c_str(), "tcp://*:12345");
}

TEST_F(LagerUtilTests, RemoteUri)
{
    ASSERT_STREQ(lager_utils::getRemoteUri("test", 12345).c_str(), "tcp://test:12345");
}

TEST_F(LagerUtilTests, ZmqVersion)
{
    ASSERT_STREQ(lager_utils::getZmqVersion().c_str(), ZMQ_VER);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
