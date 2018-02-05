#include "data_format.h"

DataFormat::DataFormat(const std::string& version_in): version(version_in), itemsSize(0)
{
}

DataFormat::~DataFormat()
{
}

void DataFormat::addItem(const DataItem& item)
{
    items.push_back(item);
    itemsSize += item.size;
}
