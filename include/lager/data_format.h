#ifndef DATA_FORMAT
#define DATA_FORMAT

#include <iostream>
#include <string>
#include <vector>

#include "lager_defines.h"

/**
 * @brief Struct to hold info about a single column of data
 */
struct DataItem
{
    std::string name; // description of data column
    std::string type; // type as a string correspoding to accepted lager formats
    size_t size; // size in bytes of the data type
    off_t offset; // offset in bytes from zero of the column

    DataItem(const std::string& n, const std::string& t, unsigned int s, unsigned int o):
        name(n), type(t), size(s), offset(o) {}
};

/**
 * @brief Data structure to store the format of a set of data
 */
class DataFormat
{
public:
    explicit DataFormat(const std::string& version, const std::string& key);
    virtual ~DataFormat();

    std::vector<DataItem> getItems() {return items;}
    unsigned int getItemCount() {return items.size();}
    std::string getVersion() {return version;}
    std::string getKey() {return key;}
    size_t getItemsSize() {return itemsSize;}

    void addItem(const DataItem& item);

    friend std::ostream& operator<<(std::ostream& stream, const DataFormat& df)
    {
        stream << "version: " << df.version << " key: " << df.key << std::endl;

        for (auto i = df.items.begin(); i != df.items.end(); ++i)
        {
            stream << (*i).name << " " << (*i).type << std::endl;
        }

        return stream;
    }

private:
    std::vector<DataItem> items;
    std::string version;
    std::string key;
    size_t itemsSize;
};

#endif
