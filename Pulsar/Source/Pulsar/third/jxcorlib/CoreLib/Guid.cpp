#include "Guid.h"
#include <memory>
#include <random>

#if WIN32
    #include <objbase.h>
#endif

namespace jxcorlib
{

    guid_t guid_t::create_new()
    {
        guid_t nguid;
        bool created = false;
#if _WIN32
        GUID guid;
        CoCreateGuid(&guid);
        constexpr int a = sizeof(long);
        if constexpr (sizeof(GUID) == sizeof(guid_t))
        {
            memcpy(&nguid, &guid, sizeof(guid_t));
            created = true;
        }
        else if constexpr (sizeof(GUID::Data1) + sizeof(GUID::Data2) + sizeof(GUID::Data3) + sizeof(GUID::Data4) == 16)
        {
            size_t offset = 0;
            memcpy(&nguid, &guid.Data1, sizeof(guid.Data1));
            offset += sizeof(guid.Data1);
            memcpy(reinterpret_cast<char*>(&nguid) + offset, &guid.Data2, sizeof(guid.Data2));
            offset += sizeof(guid.Data2);
            memcpy(reinterpret_cast<char*>(&nguid) + offset, &guid.Data3, sizeof(guid.Data3));
            offset += sizeof(guid.Data3);
            memcpy(reinterpret_cast<char*>(&nguid) + offset, &guid.Data4, sizeof(guid.Data4));
            created = true;
        }
        else
        {
            created = false;
        }
#endif

        if (!created)
        {
            static std::random_device seed;
            static std::ranlux48 engine{seed()};
            static std::uniform_int_distribution<> distrib{0, 15};

            char* v = (char*)&nguid;

            for (size_t i = 0; i < 16; i++)
            {
                v[i] = distrib(engine);
            }
        }

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
        char buffer[33];
        buffer[32] = 0;

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
} // namespace jxcorlib