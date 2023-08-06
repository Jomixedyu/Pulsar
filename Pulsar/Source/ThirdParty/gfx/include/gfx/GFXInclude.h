#pragma once

#include <memory>
#include <cstdint>
#include <type_traits>

#define GFX_DECL_SPTR(Class) using Class##_sp = std::shared_ptr<Class>;

namespace gfx
{
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