#include "data_format.h"

DataFormat::DataFormat(const std::string& version_in): version(version_in)
{
}

DataFormat::~DataFormat()
{
}

void DataFormat::addItem(const DataItem& item)
{
    items.push_back(item);
}

unsigned int DataFormat::getPayloadSize()
{
    unsigned int totalSize = 0;

    for (auto i = items.begin(); i != items.end(); ++i)
    {
        totalSize += (*i).size;
    }

    return totalSize;
}
