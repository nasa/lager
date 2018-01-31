#include "data_ref_item.h"

template<>
DataRefItem<uint8_t>::DataRefItem(const std::string& name_in, uint8_t* dataRef_in):
    AbstractDataRefItem(name_in, "uint8_t", sizeof(uint8_t)),
    dataRef(dataRef_in) {}

template<>
void DataRefItem<uint8_t>::getNetworkDataRef(void* data)
{
    *(uint8_t*)data = *reinterpret_cast<uint8_t*>(dataRef);
}

template<>
DataRefItem<int8_t>::DataRefItem(const std::string& name_in, int8_t* dataRef_in):
    AbstractDataRefItem(name_in, "int8_t", sizeof(int8_t)),
    dataRef(dataRef_in) {}

template<>
void DataRefItem<int8_t>::getNetworkDataRef(void* data)
{
    *(int8_t*)data = htons(*reinterpret_cast<int8_t*>(dataRef));
}

template<>
DataRefItem<uint16_t>::DataRefItem(const std::string& name_in, uint16_t* dataRef_in):
    AbstractDataRefItem(name_in, "uint16_t", sizeof(uint16_t)),
    dataRef(dataRef_in) {}

template<>
void DataRefItem<uint16_t>::getNetworkDataRef(void* data)
{
    *(uint16_t*)data = htons(*reinterpret_cast<uint16_t*>(dataRef));
}

template<>
DataRefItem<int16_t>::DataRefItem(const std::string& name_in, int16_t* dataRef_in):
    AbstractDataRefItem(name_in, "int16_t", sizeof(int16_t)),
    dataRef(dataRef_in) {}

template<>
void DataRefItem<int16_t>::getNetworkDataRef(void* data)
{
    *(int16_t*)data = htons(*reinterpret_cast<int16_t*>(dataRef));
}

template<>
DataRefItem<uint32_t>::DataRefItem(const std::string& name_in, uint32_t* dataRef_in):
    AbstractDataRefItem(name_in, "uint32_t", sizeof(uint32_t)),
    dataRef(dataRef_in) {}

template<>
void DataRefItem<uint32_t>::getNetworkDataRef(void* data)
{
    *(uint32_t*)data = htonl(*reinterpret_cast<uint32_t*>(dataRef));
}

template<>
DataRefItem<int32_t>::DataRefItem(const std::string& name_in, int32_t* dataRef_in):
    AbstractDataRefItem(name_in, "int32_t", sizeof(int32_t)),
    dataRef(dataRef_in) {}

template<>
void DataRefItem<int32_t>::getNetworkDataRef(void* data)
{
    *(int32_t*)data = htonl(*reinterpret_cast<int32_t*>(dataRef));
}

template<>
DataRefItem<uint64_t>::DataRefItem(const std::string& name_in, uint64_t* dataRef_in):
    AbstractDataRefItem(name_in, "uint64_t", sizeof(uint64_t)),
    dataRef(dataRef_in) {}

template<>
void DataRefItem<uint64_t>::getNetworkDataRef(void* data)
{
    //*(uint64_t*)data = lager_utils::htonll(*reinterpret_cast<uint64_t*>(dataRef));
    *(uint64_t*)data = *reinterpret_cast<uint64_t*>(dataRef);
}

template<>
DataRefItem<int64_t>::DataRefItem(const std::string& name_in, int64_t* dataRef_in):
    AbstractDataRefItem(name_in, "int64_t", sizeof(int64_t)),
    dataRef(dataRef_in) {}

template<>
void DataRefItem<int64_t>::getNetworkDataRef(void* data)
{
    *(int64_t*)data = *reinterpret_cast<int64_t*>(dataRef);
}

template<>
DataRefItem<double>::DataRefItem(const std::string& name_in, double* dataRef_in):
    AbstractDataRefItem(name_in, "double", sizeof(double)),
    dataRef(dataRef_in) {}

template<>
void DataRefItem<double>::getNetworkDataRef(void* data)
{
    *(double*)data = *reinterpret_cast<double*>(dataRef);
}


