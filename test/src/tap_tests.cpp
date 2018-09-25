#include <memory>

#include <gtest/gtest.h>

#include "lager/tap.h"
#include "lager/lager_utils.h"
#include "lager/data_ref_item.h"

TEST(TapTests, BadPortNumber)
{
    Tap t;
    EXPECT_FALSE(t.init("localhost", -50, 100));
    EXPECT_FALSE(t.init("localhost", 65536, 100));
}

TEST(TapTests, DuplicateValues)
{
    Tap t;
    int arraySize = 10;
    uint32_t uint1 = 0;

    t.init("localhost", 12345, 1000);
    t.addItem(new DataRefItem<uint32_t>("num1", &uint1));

    t.start("/test");

    for (unsigned int i = 0; i < arraySize; ++i)
    {
        t.log();
        lager_utils::sleepMillis(5);

        uint1 += 1;

        //this was the failure in the tap tests
        t.addItem(new DataRefItem<uint32_t>("num1", &uint1));
    }

    std::vector<AbstractDataRefItem*> datarefItems = t.getItems();
    std::vector<std::string> datarefNames;

    for (int i = 0; i < datarefItems.size(); i++)
    {
        datarefNames.push_back(datarefItems[i]->getName());
    }

    std::sort(datarefNames.begin(), datarefNames.end());

    std::string prevItem = "";
    std::string currItem = "";

    for (int i = 0; i < datarefNames.size(); i++)
    {
        prevItem = currItem;
        currItem = datarefNames[i];

        //skip first variable
        if (i != 0)
        {
            EXPECT_NE(currItem, prevItem);
        }
    }

    t.stop();
}

TEST(TapTests, StartWithNoDataItems)
{
    Tap t;
    t.init("localhost", 12345, 1000);
    EXPECT_ANY_THROW(t.start("/thisshouldfail"));
}

namespace tap_tests
{

    class LagerTester
    {
    public:
        LagerTester() {}
        ~LagerTester()
        {
            tap.stop();
        }

        void initTap(const std::string& hostName, int port, int timeout)
        {
            tap.init(hostName, port, timeout);
            tap.addItem(new DataRefItem<double>("val1", &val1));
            tap.addItem(new DataRefItem<double>("val2", &val2));
            tap.start("/lagerTester");
        }

        Tap tap;
    private:
        double val1;
        double val2;
    };

}

TEST(TapOwnedByClass, test)
{
    tap_tests::LagerTester tester;
    tester.initTap("localhost", 1234, 1000);
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
