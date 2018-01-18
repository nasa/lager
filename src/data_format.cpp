#include "data_format.h"

DataFormat::DataFormat(const std::string& version_in): version(version_in)
{

}

void DataFormat::addItem(const DataItem& item)
{
    items.push_back(item);
}

void DataFormat::print()
{
    std::cout << "version: " << version << std::endl;

    for (std::vector<DataItem>::iterator i = items.begin(); i != items.end(); ++i)
    {
        std::cout << (*i).name << " " << (*i).type << std::endl;
    }
}
