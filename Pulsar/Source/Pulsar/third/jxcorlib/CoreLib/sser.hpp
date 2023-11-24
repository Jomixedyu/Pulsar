#pragma once
#include <array>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <iostream>

#define _SSER
namespace sser
{
#define SSER_BUILTIN_TYPE(TYPE) inline std::iostream& ReadWriteStream(std::iostream& stream, bool write, TYPE& value) \
    { \
        if (write) stream.write((char*)&value, sizeof(value)); \
        else stream.read((char*)&value, sizeof(value)); \
        return stream;\
    }

    SSER_BUILTIN_TYPE(char);
    SSER_BUILTIN_TYPE(uint8_t);
    SSER_BUILTIN_TYPE(int8_t);
    SSER_BUILTIN_TYPE(uint16_t);
    SSER_BUILTIN_TYPE(int16_t);
    SSER_BUILTIN_TYPE(uint32_t);
    SSER_BUILTIN_TYPE(int32_t);
    SSER_BUILTIN_TYPE(uint64_t);
    SSER_BUILTIN_TYPE(int64_t);
    SSER_BUILTIN_TYPE(bool);
    SSER_BUILTIN_TYPE(float);
    SSER_BUILTIN_TYPE(double);

    inline std::iostream& ReadWriteStream(std::iostream& stream, bool write, std::string& value)
    {
        auto len = value.size();
        ReadWriteStream(stream, write, len);

        if (write)
        {
            stream.write(value.c_str(), value.size());
        }
        else //read
        {
            if (len == 0) return stream;
            value.resize(len);
            stream.read(value.data(), len);
        }
        return stream;
    }

    template<typename E> requires std::is_enum_v<E>
    inline std::iostream& ReadWriteStream(std::iostream& stream, bool write, E& value)
    {
        if (write)
            stream.write((char*)&value, sizeof(value));
        else
            stream.read((char*)&value, sizeof(value));
        return stream;
    }


    template<typename T>
    inline std::iostream& ReadWriteStream(std::iostream& stream, bool write, std::vector<T>& value)
    {
        auto len = value.size();
        ReadWriteStream(stream, write, len);
        if (!write)  //read
        {
            value.resize(len);
        }
        for (auto& item : value)
        {
            ReadWriteStream(stream, write, item);
        }
        return stream;
    }

    template<typename T, size_t N>
    inline std::iostream& ReadWriteStream(std::iostream& stream, bool write, T(&value)[N])
    {
        size_t len = N;
        ReadWriteStream(stream, write, len);
        if (!write) //read
        {
            if (len > N)
                len = N;
        }
        for (size_t i = 0; i < len; i++)
        {
            ReadWriteStream(stream, write, value[i]);
        }
        return stream;
    }

    template<typename T, size_t N>
    inline std::iostream& ReadWriteStream(std::iostream& stream, bool write, std::array<T, N>& value)
    {
        int len = N;
        ReadWriteStream(stream, write, len);
        if (!write) //read
        {
            if (len > N)
                len = N;
        }
        for (int i = 0; i < len; i++)
        {
            ReadWriteStream(stream, write, value[i]);
        }
        return stream;
    }

    template<typename K, typename V>
    inline std::iostream& ReadWriteStream(std::iostream& stream, bool write, std::unordered_map<K, V>& value)
    {
        auto len = value.size();
        ReadWriteStream(stream, write, len);
        if (write)
        {
            for (auto& [k, v] : value)
            {
                auto _k = k;
                ReadWriteStream(stream, write, _k);
                ReadWriteStream(stream, write, v);
            }
        }
        else //read
        {
            value.reserve(len);
            for (size_t i = 0; i < len; i++)
            {
                K k{};
                V v{};
                ReadWriteStream(stream, write, k);
                ReadWriteStream(stream, write, v);

                value.insert({ k,v });
            }
        }
        return stream;
    }

#undef SSER_BUILTIN_TYPE
}