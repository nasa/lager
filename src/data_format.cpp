#include "lager/data_format.h"

/**
 * @brief Constructor, version required
 * @param version string containing the format version
 */
DataFormat::DataFormat(const std::string& version_in, const std::string& key_in): version(version_in), key(key_in), itemsSize(0)
{
}

DataFormat::~DataFormat()
{
}

/**
 * @brief Adds a data item to the list
 * @param item is a DataItem created by the user to add to the list
 */
void DataFormat::addItem(const DataItem& item)
{
    items.push_back(item);
    itemsSize += item.size;
}
