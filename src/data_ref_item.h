#ifndef DATA_REF_ITEM
#define DATA_REF_ITEM

#include <string>
#include <stdint.h>
#include <typeinfo>
#include <stdexcept>
#include <arpa/inet.h>

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

#endif
