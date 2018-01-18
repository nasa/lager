#include <iostream>

#include "data_format.h"
#include "data_format_parser.h"

using namespace xercesc;

int main(int argc, char* argv[])
{
    DataFormatParser p("sample_format.xml", "data_format.xsd");
    std::shared_ptr<DataFormat> df = p.parse();
    df->print();

    return 0;
}
