#pragma once

#define ENUM_CLASS_FLAGS(Enum) \
inline constexpr Enum& operator|=(Enum& x, Enum y) { \
    return x = static_cast<Enum>( \
        static_cast<std::underlying_type_t<Enum>>(x) | static_cast<std::underlying_type_t<Enum>>(y) \
); } \
inline constexpr Enum& operator&=(Enum& x, Enum y) { \
    return x = static_cast<Enum>( \
        static_cast<std::underlying_type_t<Enum>>(x) & static_cast<std::underlying_type_t<Enum>>(y) \
); } \
inline constexpr Enum& operator^=(Enum& x, Enum y) { \
    return x = static_cast<Enum>( \
        static_cast<std::underlying_type_t<Enum>>(x) ^ static_cast<std::underlying_type_t<Enum>>(y) \
); } \
inline constexpr Enum operator| (Enum x, Enum y) { \
    return static_cast<Enum>( \
        static_cast<std::underlying_type_t<Enum>>(x) | static_cast<std::underlying_type_t<Enum>>(y) \
); } \
inline constexpr Enum operator& (Enum x, Enum y) { \
    return static_cast<Enum>( \
        static_cast<std::underlying_type_t<Enum>>(x) & static_cast<std::underlying_type_t<Enum>>(y) \
); } \
inline constexpr Enum operator^ (Enum x, Enum y) { \
    return static_cast<Enum>( \
        static_cast<std::underlying_type_t<Enum>>(x) ^ static_cast<std::underlying_type_t<Enum>>(y) \
); } \
inline constexpr bool operator! (Enum x) { \
    return !static_cast<std::underlying_type_t<Enum>>(x); \
} \
inline constexpr Enum operator~ (Enum x) { \
    return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(x)); \
}

template<typename Enum>
constexpr inline bool EnumHasFlag(Enum a, Enum b)
{
    return static_cast<bool>( static_cast<std::underlying_type_t<Enum>>(a & b) );
}