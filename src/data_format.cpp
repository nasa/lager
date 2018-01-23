#include "data_format.h"

DataFormat::DataFormat(const std::string& version_in): version(version_in)
{

}

void DataFormat::addItem(const DataItem& item)
{
    items.push_back(item);
}

void DataFormat::print(std::ostream& stream)
{
    stream << "version: " << version << std::endl;

    for (std::vector<DataItem>::iterator i = items.begin(); i != items.end(); ++i)
    {
        stream << (*i).name << " " << (*i).type << std::endl;
    }
}

unsigned int DataFormat::getPayloadSize()
{
    unsigned int totalSize = 0;

    for (std::vector<DataItem>::iterator i = items.begin(); i != items.end(); ++i)
    {
        totalSize += (*i).size;
    }

    return totalSize;
}
