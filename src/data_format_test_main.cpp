#include <iostream>

#include "data_format.h"
#include "data_format_parser.h"

int main(int argc, char* argv[])
{
    try
    {
        DataFormatParser p("sample_format.xml", "data_format.xsd");

        std::shared_ptr<DataFormat> df = p.parse();
        df->print();
    }
    catch (const std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
