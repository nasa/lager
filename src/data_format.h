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
    virtual ~DataFormat();

    std::vector<DataItem> getItems() {return items;}
    unsigned int getItemCount() {return items.size();}
    std::string getVersion() {return version;}

    unsigned int getPayloadSize();
    void addItem(const DataItem& item);

    friend std::ostream& operator<<(std::ostream& stream, const DataFormat& df)
    {
        stream << "version: " << df.version << std::endl;

        for (auto i = df.items.begin(); i != df.items.end(); ++i)
        {
            stream << (*i).name << " " << (*i).type << std::endl;
        }

        return stream;
    }

private:
    std::vector<DataItem> items;
    std::string version;
};

#endif
