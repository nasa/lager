#ifndef DATA_ITEM
#define DATA_ITEM

#include <stdint.h>

class DataRefItem
{
public:
    DataRefItem(const std::string& n, uint32_t& dataRef_in);

    uint32_t getSize() {return size;};
    void* getData() {return (void*)&dataRef;};

private:
    uint32_t& dataRef;
    std::string name;
    std::string type;
    uint32_t size;
    uint32_t offset;
};

#endif
