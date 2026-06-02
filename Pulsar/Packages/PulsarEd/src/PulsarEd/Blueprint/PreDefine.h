#pragma once

#define PRE_CORELIB_NO_GUID_TYPE
#include <CoreLib/Guid.h>

#include <vector>
#include <memory>

namespace bp
{
    using guid_t = jxcorlib::guid_t;

    template<typename T>
    using array_list = std::vector<T>;

    using string = std::string;
}
