#include <memory>

#include <gtest/gtest.h>

#include "lager/tap.h"
#include "lager/lager_utils.h"
#include "lager/data_ref_item.h"

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

TEST_F(TapTests, BadPortNumber)
{
    Tap t;
    EXPECT_FALSE(t.init("localhost", -50, 100));
    EXPECT_FALSE(t.init("localhost", 65536, 100));
}

float ntohf(float nf)
{
   float x;
   nf = ntohl(nf);
   memcpy(&x, &nf, sizeof(float));
   return x;
}

TEST_F(TapTests, DuplicateValues) //This is to demonstrate the tap tests were having before
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
        t.addItem(new DataRefItem<uint32_t>("num1", &uint1)); //this was the failure in the tap tests
    }    

    std::vector<AbstractDataRefItem*> datarefitems = t.getItems();
    float prevUnpackedValue = 0.0;
    float theUnpackedValue = 0.0;
    for(int i = 0; i < datarefitems.size(); i++) 
    {
        prevUnpackedValue = theUnpackedValue;
        float bigEndianValue;
        memcpy(&bigEndianValue, &datarefitems[0], sizeof(bigEndianValue));
        theUnpackedValue = ntohf(bigEndianValue);
        if (prevUnpackedValue == theUnpackedValue) 
        {
            t.stop();
            std::cerr<<"Duplicate values detected."<<std::endl;
            EXPECT_FALSE(false);
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
