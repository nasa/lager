#ifndef DATA_REF_ITEM
#define DATA_REF_ITEM

#include <string>
#include <stdint.h>
#include <typeinfo>
#include <stdexcept>

#include "lager/lager_utils.h"

/**
 * @brief Abstract class object used to define a column of data in the data format
 */
struct AbstractDataRefItem
{
    AbstractDataRefItem(const std::string& name_in, const std::string& type_in, size_t size_in):
        name(name_in), type(type_in), size(size_in), offset(0) {}

    virtual void getNetworkDataRef(void* data) = 0;

    const std::string getName() {return name;}
    const std::string getType() {return type;}
    size_t getSize() {return size;}
    off_t getOffset() {return offset;}
    void setOffset(off_t offset_in) {offset = offset_in;}

protected:
    std::string name; /*!< a descriptive name for this column of data */
    std::string type; /*!< a lager data type */
    size_t size; /*!< the size of the data type */
    off_t offset; /*!< the offset of this column of data from zero */
};

/**
 * @brief Main template overload of the abstract item, not intended for actual use.
 * Use one of the specialized templates below.
 */
template<class T>
class DataRefItem : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, T* dataRef_in):
        AbstractDataRefItem(name_in, "unsupported", sizeof(T)),
        dataRef(dataRef_in)
    {
        throw std::runtime_error("unsupported data type");
    }

    ~DataRefItem() {}

    void getNetworkDataRef(void* data)
    {
        throw std::runtime_error("unsupported data type");
    }

private:
    T* dataRef;
};

template<> inline DataRefItem<uint8_t>::DataRefItem(const std::string& name_in, uint8_t* dataRef_in):
    AbstractDataRefItem(name_in, "uint8_t", sizeof(uint8_t)),
    dataRef(dataRef_in) {}

template<> inline void DataRefItem<uint8_t>::getNetworkDataRef(void* data)
{
    *(uint8_t*)data = *reinterpret_cast<uint8_t*>(dataRef);
}

template<> inline DataRefItem<int8_t>::DataRefItem(const std::string& name_in, int8_t* dataRef_in):
    AbstractDataRefItem(name_in, "int8_t", sizeof(int8_t)),
    dataRef(dataRef_in) {}

template<> inline void DataRefItem<int8_t>::getNetworkDataRef(void* data)
{
    *(int8_t*)data = *reinterpret_cast<int8_t*>(dataRef);
}

template<> inline DataRefItem<uint16_t>::DataRefItem(const std::string& name_in, uint16_t* dataRef_in):
    AbstractDataRefItem(name_in, "uint16_t", sizeof(uint16_t)),
    dataRef(dataRef_in) {}

template<> inline void DataRefItem<uint16_t>::getNetworkDataRef(void* data)
{
    *(uint16_t*)data = htons(*reinterpret_cast<uint16_t*>(dataRef));
}

template<> inline DataRefItem<int16_t>::DataRefItem(const std::string& name_in, int16_t* dataRef_in):
    AbstractDataRefItem(name_in, "int16_t", sizeof(int16_t)),
    dataRef(dataRef_in) {}

template<> inline void DataRefItem<int16_t>::getNetworkDataRef(void* data)
{
    *(uint16_t*)data = htons(*reinterpret_cast<uint16_t*>(dataRef));
}

template<> inline DataRefItem<uint32_t>::DataRefItem(const std::string& name_in, uint32_t* dataRef_in):
    AbstractDataRefItem(name_in, "uint32_t", sizeof(uint32_t)),
    dataRef(dataRef_in) {}

template<> inline void DataRefItem<uint32_t>::getNetworkDataRef(void* data)
{
    *(uint32_t*)data = htonl(*reinterpret_cast<uint32_t*>(dataRef));
}

template<> inline DataRefItem<float>::DataRefItem(const std::string& name_in, float* dataRef_in):
    AbstractDataRefItem(name_in, "float32", sizeof(float)),
    dataRef(dataRef_in) {}

template<> inline void DataRefItem<float>::getNetworkDataRef(void* data)
{
    *(uint32_t*)data = htonl(*reinterpret_cast<uint32_t*>(dataRef));
}

template<> inline DataRefItem<int32_t>::DataRefItem(const std::string& name_in, int32_t* dataRef_in):
    AbstractDataRefItem(name_in, "int32_t", sizeof(int32_t)),
    dataRef(dataRef_in) {}

template<> inline void DataRefItem<int32_t>::getNetworkDataRef(void* data)
{
    *(uint32_t*)data = htonl(*reinterpret_cast<uint32_t*>(dataRef));
}

template<> inline DataRefItem<uint64_t>::DataRefItem(const std::string& name_in, uint64_t* dataRef_in):
    AbstractDataRefItem(name_in, "uint64_t", sizeof(uint64_t)),
    dataRef(dataRef_in) {}

template<> inline void DataRefItem<uint64_t>::getNetworkDataRef(void* data)
{
    *(uint64_t*)data = lager_utils::htonll(*reinterpret_cast<uint64_t*>(dataRef));
}

template<> inline DataRefItem<int64_t>::DataRefItem(const std::string& name_in, int64_t* dataRef_in):
    AbstractDataRefItem(name_in, "int64_t", sizeof(int64_t)),
    dataRef(dataRef_in) {}

template<> inline void DataRefItem<int64_t>::getNetworkDataRef(void* data)
{
    *(uint64_t*)data = lager_utils::htonll(*reinterpret_cast<uint64_t*>(dataRef));
}

template<> inline DataRefItem<double>::DataRefItem(const std::string& name_in, double* dataRef_in):
    AbstractDataRefItem(name_in, "float64", sizeof(double)),
    dataRef(dataRef_in) {}

template<> inline void DataRefItem<double>::getNetworkDataRef(void* data)
{
    *(uint64_t*)data = lager_utils::htonll(*reinterpret_cast<uint64_t*>(dataRef));
}

#endif
