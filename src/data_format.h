#ifndef DATA_FORMAT
#define DATA_FORMAT

#include <iostream>
#include <string>
#include <vector>

#include "lager_defines.h"

struct DataItem
{
    std::string name;
    std::string type;
    unsigned int size;
    unsigned int offset;

    DataItem(const std::string& n, const std::string& t, unsigned int s, unsigned int o):
        name(n), type(t), size(s), offset(o) {}
};

class DataFormat
{
public:
    DataFormat(const std::string& version);

    std::vector<DataItem> getItems() {return items;};
    std::string getVersion() {return version;};

    unsigned int getPayloadSize();
    void addItem(const DataItem& item);
    void print(std::ostream& stream);

private:
    std::vector<DataItem> items;
    std::string version;
};

#endif
