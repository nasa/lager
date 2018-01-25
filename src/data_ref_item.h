#ifndef DATA_REF_ITEM
#define DATA_REF_ITEM

#include <string>
#include <stdint.h>
#include <typeinfo>

struct AbstractDataRefItem
{
    virtual uint32_t getSize() = 0;
    virtual uint32_t getOffset() = 0;
    virtual void setOffset(uint32_t offset_in) = 0;
    virtual const std::string getName() = 0;
    virtual const std::string getType() = 0;
    virtual void* getData() = 0;
};

template<class T>
class DataRefItem : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, T& dataRef_in):
        name(name_in), dataRef(dataRef_in), size(sizeof(dataRef_in))
    {
        size_t typeCode = typeid(T).hash_code();

        if (typeCode == typeid(uint32_t).hash_code())
        {
            type = std::string("uint32_t");
        }
        else
        {
            throw std::runtime_error("unsupported data type");
        }
    }

    ~DataRefItem() {}

    const std::string getName() {return name;}
    const std::string getType() {return type;}
    uint32_t getSize() {return size;}
    uint32_t getOffset() {return offset;}
    void setOffset(uint32_t offset_in) {offset = offset_in;}
    void* getData() {return (void*)&dataRef;}

private:
    T& dataRef;
    std::string name;
    std::string type;
    uint32_t size;
    uint32_t offset;
};

#endif
