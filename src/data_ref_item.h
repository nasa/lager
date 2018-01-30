#ifndef DATA_REF_ITEM
#define DATA_REF_ITEM

#include <string>
#include <stdint.h>
#include <typeinfo>

#include "lager_utils.h"

struct AbstractDataRefItem
{
    virtual size_t getSize() = 0;
    virtual off_t getOffset() = 0;
    virtual void setOffset(off_t offset_in) = 0;
    virtual void getNetworkDataRef(void* data) = 0;
    virtual const std::string getName() = 0;
    virtual const std::string getType() = 0;
};

template<class T>
class DataRefItem : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, T* dataRef_in):
        name(name_in), dataRef(dataRef_in), size(sizeof(dataRef_in))
    {
        size_t typeCode = typeid(T).hash_code();

        if (typeCode == typeid(uint32_t).hash_code())
        {
            type = std::string("uint32_t");
        }
        else if (typeCode == typeid(int32_t).hash_code())
        {
            type = std::string("int32_t");
        }
        else if (typeCode == typeid(double).hash_code())
        {
            type = std::string("double");
        }
        else
        {
            throw std::runtime_error("unsupported data type");
        }
    }

    ~DataRefItem() {}

    const std::string getName() {return name;}
    const std::string getType() {return type;}
    size_t getSize() {return size;}
    off_t getOffset() {return offset;}
    void setOffset(off_t offset_in) {offset = offset_in;}

    void getNetworkDataRef(void* data)
    {
        switch (size)
        {
            case 1:
                *(uint8_t*)data = *dataRef;
                break;

            case 2:
                *(uint16_t*)data = htons(*reinterpret_cast<uint16_t*>(dataRef));
                break;

            case 4:
                *(uint32_t*)data = htonl(*reinterpret_cast<uint32_t*>(dataRef));
                break;

            case 8:
                *(uint64_t*)data = lager_utils::htonll(*reinterpret_cast<uint64_t*>(dataRef));
                break;

            default:
                throw std::runtime_error("unsupported data size");
        }
    }

private:
    T* dataRef;
    std::string name;
    std::string type;
    size_t size;
    off_t offset;
};

#endif
