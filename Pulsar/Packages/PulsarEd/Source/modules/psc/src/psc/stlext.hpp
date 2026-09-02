#pragma once
#include <algorithm>

namespace stlext
{
    template<typename T, typename K>
    bool contains(const T& c, const K& key)
    {
        return std::find(c.begin(), c.end(), key) != c.end();
    }
}