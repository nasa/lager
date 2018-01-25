#ifndef DATA_REF_ITEM
#define DATA_REF_ITEM

#include <string>
#include <stdint.h>

struct AbstractDataRefItem
{
    virtual uint32_t getSize() = 0;
    virtual void* getData() = 0;
};

template<class T>
class DataRefItem : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, T& dataRef_in):
        name(name_in), dataRef(dataRef_in), size(sizeof(dataRef_in)) {}
    ~DataRefItem() {}

    const std::string getName() {return name;}
    const std::string getType() {return type;}
    uint32_t getSize() {return size;}
    void* getData() {return (void*)&dataRef;}

private:
    T& dataRef;
    std::string name;
    std::string type;
    uint32_t size;
    uint32_t offset;
};

#endif
