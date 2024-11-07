#pragma once

#include <memory>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>

#define GFX_DECL_SPTR(Class) using Class##_sp = std::shared_ptr<Class>; using Class##_wp = std::weak_ptr<Class>;

namespace gfx
{
    template<typename T>
    using array_list = std::vector<T>;

    template<typename T>
    inline std::shared_ptr<T> gfxmksptr(T* ptr)
    {
        return std::shared_ptr<T>(ptr);
    }

    template<typename T>
    inline T AndFlag(T a, T b)
    {
        return T(std::underlying_type_t<T>(a) & std::underlying_type_t<T>(b));
    }

    template<typename T>
    inline T OrFlag(T a, T b)
    {
        return T(std::underlying_type_t<T>(a) | std::underlying_type_t<T>(b));
    }

    template<typename T>
    inline bool HasFlag(T a, T b)
    {
        return (bool)std::underlying_type_t<T>(AndFlag(a, b));
    }
}