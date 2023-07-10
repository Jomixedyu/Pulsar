#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace gfx
{
    using std::string;
    using std::string_view;
    template<typename T>
    using array_list = std::vector<T>;

}