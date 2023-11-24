#include "Guid.h"
#include <random>
#include <memory>

#if WIN32
#include <objbase.h>
#endif

namespace jxcorlib
{

    guid_t guid_t::create_new()
    {
        guid_t nguid;
#if WIN32
        static_assert(sizeof(GUID) == sizeof(guid_t));
        GUID guid;
        if (CoCreateGuid(&guid) == S_OK)
        {
            memcpy(&nguid, &guid, sizeof(guid_t));
        }
#else
        static std::random_device seed;
        static std::ranlux48 engine{ seed() };
        static std::uniform_int_distribution<> distrib{ 0, 15 };

        char* v = (char*)&nguid;

        for (size_t i = 0; i < 16; i++)
        {
            v[i] = distrib(engine);
        }
#endif
        return nguid;
    }

    static const char* hexlist = "0123456789abcdef";
    static void bytetohex(uint8_t value, char* out_str)
    {
        out_str[0] = hexlist[value / 16];
        out_str[1] = hexlist[value % 16];
        out_str[2] = 0;
    }


    static uint8_t char_to_num(char c)
    {
        if (c >= 'A' && c <= 'Z')
        {
            return c - 'A' + 10;
        }
        else if (c >= 'a' && c <= 'z')
        {
            return c - 'a' + 10;
        }
        else if (c >= '0' && c <= '9')
        {
            return c - '0';
        }
        return 0;
    }

    static uint8_t hex_to_byte(const char* str)
    {
        return char_to_num(str[0]) * 16 + char_to_num(str[1]);
    }

    guid_t guid_t::parse(std::string_view str)
    {
        guid_t guid;
        char buffer[33]; buffer[32] = 0;

        if (str.length() == 32)
        {
            ::memcpy(buffer, str.data(), 32);
        }
        else if (str.length() == 32 + 4) /* 8-4-4-4-12 */
        {
            ::memcpy(buffer, str.data(), 8);
            ::memcpy(buffer + 8, str.data() + 8 + 1, 4);
            ::memcpy(buffer + 12, str.data() + 12 + 2, 4);
            ::memcpy(buffer + 16, str.data() + 16 + 3, 4);
            ::memcpy(buffer + 20, str.data() + 20 + 4, 12);
        }
        else
        {
            return guid;
        }

        for (size_t i = 0; i < 16; i++)
        {
            *((uint8_t*)&guid.x + i) = hex_to_byte(buffer + i * 2);
        }
        return guid;
    }

    bool guid_t::is_empty() const
    {
        return !(x || y || z || w);
    }

    bool guid_t::operator==(guid_t right) const
    {
        return x == right.x && y == right.y && z == right.z && w == right.w;
    }

    guid_t::operator bool() const
    {
        return !this->is_empty();
    }

    std::string guid_t::to_string() const
    {
        std::string str;
        str.reserve(16);
        char hex[3];
        for (size_t i = 0; i < 16; i++)
        {
            bytetohex(*((uint8_t*)this + i), hex);
            str.append(hex);
        }
        return str;
    }
}