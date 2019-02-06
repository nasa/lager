#include <memory>

#include <gtest/gtest.h>

#include "lager/keg.h"
#include "lager/lager_utils.h"

class KegTests : public ::testing::Test
{
protected:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {

    }
};

TEST_F(KegTests, BadDir)
{
    EXPECT_ANY_THROW(Keg k("hahathisisntadirectory"));
}

TEST_F(KegTests, StartAlreadyRunning)
{
    Keg k(".");

    k.start();
    EXPECT_ANY_THROW(k.start());
}

TEST_F(KegTests, StopNotRunning)
{
    Keg k(".");

    EXPECT_ANY_THROW(k.stop());
}

TEST_F(KegTests, BadFormat)
{
    Keg k(".");

    EXPECT_ANY_THROW(k.addFormat("076ac37b-83dd-4fef-bc9d-16789794be87", "this_is_not_valid_xml"));
}

TEST_F(KegTests, DuplicateUuid)
{
    Keg k(".");

    k.addFormat("076ac37b-83dd-4fef-bc9d-16789794be87", "<?xml version=\"1.0\" encoding=\"UTF-8\"?><format version=\"BEERR01\" key=\"test\">"
                "<item name=\"column1\" type=\"uint32_t\" size=\"4\" offset=\"0\"/>"
                "<item name=\"column2\" type=\"uint16_t\" size=\"2\" offset=\"4\"/></format>");
    EXPECT_ANY_THROW(k.addFormat("076ac37b-83dd-4fef-bc9d-16789794be87", "<?xml version=\"1.0\" encoding=\"UTF-8\"?><format version=\"BEERR01\" key=\"test\">"
                                 "<item name=\"column1\" type=\"uint16_t\" size=\"2\" offset=\"0\"/>"
                                 "<item name=\"column2\" type=\"uint16_t\" size=\"2\" offset=\"2\"/></format>"));
}

TEST_F(KegTests, MetaData)
{
    Keg k(".");

    k.addFormat("076ac37b-83dd-4fef-bc9d-16789794be87", "<?xml version=\"1.0\" encoding=\"UTF-8\"?><format version=\"BEERR01\" key=\"test\">"
                "<item name=\"column1\" type=\"uint32_t\" size=\"4\" offset=\"0\"/>"
                "<item name=\"column2\" type=\"uint16_t\" size=\"2\" offset=\"4\"/></format>");

    k.setMetaData("testkey", "testvalue");
    k.start();
    lager_utils::sleepMillis(100);
    k.stop();
}

// make sure the format file is created and subsequentely deleted
TEST_F(KegTests, FormatFile)
{
    Keg k(".");

    k.addFormat("076ac37b-83dd-4fef-bc9d-16789794be87", "<?xml version=\"1.0\" encoding=\"UTF-8\"?><format version=\"BEERR01\" key=\"test\">"
                "<item name=\"column1\" type=\"uint32_t\" size=\"4\" offset=\"0\"/>"
                "<item name=\"column2\" type=\"uint16_t\" size=\"2\" offset=\"4\"/></format>");

    k.start();
    lager_utils::sleepMillis(100);
    EXPECT_TRUE(keg_utils::doesFileExist(k.getFormatFile()));
    k.stop();
    lager_utils::sleepMillis(100);
    EXPECT_FALSE(keg_utils::doesFileExist(k.getFormatFile()));
}

TEST_F(KegTests, DoesItWork)
{
    Keg k(".");

    k.addFormat("076ac37b-83dd-4fef-bc9d-16789794be87", "<?xml version=\"1.0\" encoding=\"UTF-8\"?><format version=\"BEERR01\" key=\"test\">"
                "<item name=\"column1\" type=\"uint32_t\" size=\"4\" offset=\"0\"/>"
                "<item name=\"column2\" type=\"uint16_t\" size=\"2\" offset=\"4\"/></format>");

    k.start();

    std::string uuid = lager_utils::getUuid("076ac37b-83dd-4fef-bc9d-16789794be87");
    uint64_t timestamp = lager_utils::getCurrentTime();
    uint32_t column1 = 12345;
    uint16_t column2 = 54321;

    std::vector<uint8_t> data;
    off_t offset = 0;

    for (size_t i = 0; i < uuid.size(); ++i)
    {
        data.push_back(uuid[i]);
    }

    offset += UUID_SIZE_BYTES;

    data.resize(data.size() + TIMESTAMP_SIZE_BYTES);
    *(reinterpret_cast<uint64_t*>(data.data() + offset)) = lager_utils::htonll(timestamp);

    offset += TIMESTAMP_SIZE_BYTES;

    data.resize(data.size() + 4);
    *(reinterpret_cast<uint32_t*>(data.data() + offset)) = htonl(column1);
    offset += 4;

    data.resize(data.size() + 2);
    *(reinterpret_cast<uint16_t*>(data.data() + offset)) = htons(column2);

    k.write(data, data.size());

    k.stop();
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
