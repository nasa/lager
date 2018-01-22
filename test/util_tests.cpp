#include <gtest/gtest.h>

#include "lager_utils.h"

class LagerUtilTests : public ::testing::Test
{};

TEST_F(LagerUtilTests, UuidLength)
{
#ifndef _WIN32
    ASSERT_EQ(lager_utils::getUuid().size(), 16);
#endif _WIN32
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
