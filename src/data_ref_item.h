#ifndef DATA_REF_ITEM
#define DATA_REF_ITEM

#include <string>
#include <stdint.h>
#include <typeinfo>

#include "lager_utils.h"

struct AbstractDataRefItem
{
    AbstractDataRefItem(const std::string& name_in, const std::string& type_in, off_t size_in):
        name(name_in), type(type_in), size(size_in) {}

    virtual void getNetworkDataRef(void* data) = 0;

    const std::string getName() {return name;}
    const std::string getType() {return type;}
    size_t getSize() {return size;}
    off_t getOffset() {return offset;}
    void setOffset(off_t offset_in) {offset = offset_in;}

protected:
    std::string name;
    std::string type;
    size_t size;
    off_t offset;
};

template<class T>
class DataRefItem : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, T* dataRef_in):
        AbstractDataRefItem(name_in, sizeof(T)),
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

template<>
class DataRefItem<uint8_t> : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, uint8_t* dataRef_in):
        AbstractDataRefItem(name_in, "uint8_t", sizeof(uint8_t)),
        dataRef(dataRef_in) {}

    ~DataRefItem() {}

    void getNetworkDataRef(void* data)
    {
        *(uint8_t*)data = *reinterpret_cast<uint8_t*>(dataRef);
    }

private:
    uint8_t* dataRef;
};

template<>
class DataRefItem<int8_t> : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, int8_t* dataRef_in):
        AbstractDataRefItem(name_in, "int8_t", sizeof(int8_t)),
        dataRef(dataRef_in) {}

    ~DataRefItem() {}

    void getNetworkDataRef(void* data)
    {
        *(int8_t*)data = *reinterpret_cast<int8_t*>(dataRef);
    }

private:
    int8_t* dataRef;
};

template<>
class DataRefItem<uint16_t> : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, uint16_t* dataRef_in):
        AbstractDataRefItem(name_in, "uint16_t", sizeof(uint16_t)),
        dataRef(dataRef_in) {}

    ~DataRefItem() {}

    void getNetworkDataRef(void* data)
    {
        *(uint16_t*)data = htons(*reinterpret_cast<uint16_t*>(dataRef));
    }

private:
    uint16_t* dataRef;
};

template<>
class DataRefItem<int16_t> : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, int16_t* dataRef_in):
        AbstractDataRefItem(name_in, "int16_t", sizeof(int16_t)),
        dataRef(dataRef_in) {}

    ~DataRefItem() {}

    void getNetworkDataRef(void* data)
    {
        *(int16_t*)data = htons(*reinterpret_cast<int16_t*>(dataRef));
    }

private:
    int16_t* dataRef;
};

template<>
class DataRefItem<uint32_t> : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, uint32_t* dataRef_in):
        AbstractDataRefItem(name_in, "uint32_t", sizeof(uint32_t)),
        dataRef(dataRef_in) {}

    ~DataRefItem() {}

    void getNetworkDataRef(void* data)
    {
        *(uint32_t*)data = htonl(*reinterpret_cast<uint32_t*>(dataRef));
    }

private:
    uint32_t* dataRef;
};

template<>
class DataRefItem<int32_t> : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, int32_t* dataRef_in):
        AbstractDataRefItem(name_in, "int32_t", sizeof(int32_t)),
        dataRef(dataRef_in) {}

    ~DataRefItem() {}

    void getNetworkDataRef(void* data)
    {
        *(int32_t*)data = htonl(*reinterpret_cast<int32_t*>(dataRef));
    }

private:
    int32_t* dataRef;
};

template<>
class DataRefItem<float> : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, float* dataRef_in):
        AbstractDataRefItem(name_in, "float", sizeof(float)),
        dataRef(dataRef_in) {}

    ~DataRefItem() {}

    void getNetworkDataRef(void* data)
    {
        *(uint32_t*)data = htonl(*reinterpret_cast<uint32_t*>(dataRef));
    }

private:
    float* dataRef;
};

template<>
class DataRefItem<uint64_t> : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, uint64_t* dataRef_in):
        AbstractDataRefItem(name_in, "uint64_t", sizeof(uint64_t)),
        dataRef(dataRef_in) {}

    ~DataRefItem() {}

    void getNetworkDataRef(void* data)
    {
        *(uint64_t*)data = lager_utils::htonll(*reinterpret_cast<uint64_t*>(dataRef));
    }

private:
    uint64_t* dataRef;
};

template<>
class DataRefItem<int64_t> : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, int64_t* dataRef_in):
        AbstractDataRefItem(name_in, "int64_t", sizeof(int64_t)),
        dataRef(dataRef_in) {}

    ~DataRefItem() {}

    void getNetworkDataRef(void* data)
    {
        *(int64_t*)data = lager_utils::htonll(*reinterpret_cast<int64_t*>(dataRef));
    }

private:
    int64_t* dataRef;
};

template<>
class DataRefItem<double> : public AbstractDataRefItem
{
public:
    DataRefItem(const std::string& name_in, double* dataRef_in):
        AbstractDataRefItem(name_in, "double", sizeof(double)),
        dataRef(dataRef_in) {}

    ~DataRefItem() {}

    void getNetworkDataRef(void* data)
    {
        *(int64_t*)data = lager_utils::htonll(*reinterpret_cast<int64_t*>(dataRef));
    }

private:
    double* dataRef;
};

#endif
