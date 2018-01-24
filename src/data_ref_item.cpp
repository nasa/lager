#include <stdint.h>
#include <string>

#include "data_ref_item.h"

DataRefItem::DataRefItem(const std::string& name_in, uint32_t& dataRef_in):
    name(name_in), dataRef(dataRef_in), size(sizeof(dataRef_in))
{
}
