#include <gtest/gtest.h>

#include "lager/lager_defines.h"
#include "lager/lager_utils.h"

class LagerUtilTests : public ::testing::Test
{};

TEST_F(LagerUtilTests, UuidLength)
{
    ASSERT_EQ(lager_utils::getUuid().size(), UUID_SIZE_BYTES);
}

TEST_F(LagerUtilTests, UuidParseAndBack)
{
    std::string uuid = lager_utils::getUuid();
    std::string uuidStr = lager_utils::getUuidString(uuid);
    std::string uuid2 = lager_utils::getUuid(uuidStr);
    ASSERT_STREQ(uuid.c_str(), uuid2.c_str());
}

TEST_F(LagerUtilTests, LocalUri)
{
    ASSERT_STREQ(lager_utils::getLocalUri(12345).c_str(), "tcp://*:12345");
}

TEST_F(LagerUtilTests, RemoteUri)
{
    ASSERT_STREQ(lager_utils::getRemoteUri("test", 12345).c_str(), "tcp://test:12345");
}

TEST_F(LagerUtilTests, GetTime)
{
    std::string gmtTime = lager_utils::getCurrentTimeFormatted("%Y%m%d", false);
    std::string localTime = lager_utils::getCurrentTimeFormatted("%Y%m%d");
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
