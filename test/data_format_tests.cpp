#include <memory>

#include <gtest/gtest.h>

#include "data_format.h"
#include "data_format_parser.h"

class DataFormatTests : public ::testing::Test
{
protected:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {

    }
};

TEST_F(DataFormatTests, GoodFormatParse)
{
    DataFormatParser p("good_format.xml", "good_schema.xsd");
    EXPECT_NO_THROW(p.parse());
}

TEST_F(DataFormatTests, BadFormatParse)
{
    DataFormatParser a("missing_items_format.xml", "good_schema.xsd");
    DataFormatParser b("good_format.xml", "bad_schema.xsd");
    DataFormatParser c("missing_name_format.xml", "good_schema.xsd");
    DataFormatParser d("missing_name_format.xml", "bad_schema.xsd");

    EXPECT_ANY_THROW(a.parse());
    EXPECT_ANY_THROW(b.parse());
    EXPECT_ANY_THROW(c.parse());
    EXPECT_ANY_THROW(d.parse());
}
