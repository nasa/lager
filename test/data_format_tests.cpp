#include <memory>

#include <gtest/gtest.h>

#include "data_format.h"
#include "data_format_parser.h"

class DataFormatTests : public ::testing::Test {};

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

TEST_F(DataFormatTests, Print)
{
    DataFormatParser p("good_format.xml", "good_schema.xsd");
    std::shared_ptr<DataFormat> df = p.parse();
    std::stringstream ss;
    df->print(ss);
    EXPECT_STREQ(ss.str().c_str(), "version: BEERR01\ncolumn1 string\ncolumn2 integer\n");
}
