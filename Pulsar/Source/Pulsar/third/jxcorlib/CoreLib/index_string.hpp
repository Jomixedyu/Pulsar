/*
MIT License

Copyright (c) 2021 JomiXedYu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include <cstring>

struct index_string_block
{
    void* bytes;
    size_t size;
};

struct __index_string_manager final
{
    static auto GetIndexStringMap()
    {
        auto static map = new std::unordered_map<size_t, index_string_block>;
        //96kb
        map->reserve(4096);
        return map;
    }
};

template<typename T>
struct basic_index_string
{
protected:
    using char_t = T;
    using index_t = size_t;
    constexpr static index_t none = 0;
protected:
    static constexpr size_t ELFHash(const char_t* str, size_t size)
    {
        constexpr size_t TotalBits = sizeof(size_t) * 8;
        constexpr size_t ThreeQuarters = (TotalBits * 3) / 4;
        constexpr size_t OneEighth = TotalBits / 8;
        constexpr size_t HighBits = ((size_t)-1) << (TotalBits - OneEighth);
        size_t hash = 0;
        size_t magic = 0;
        for (size_t i = 0; i < size; ++i)
        {
            const size_t ch = str[i];
            hash = (hash << OneEighth) + ch;
            if ((magic = hash & HighBits) != 0)
            {
                hash ^= (magic >> ThreeQuarters);
                hash &= ~magic;
            }
        }
        return hash;
    }
public:
    constexpr basic_index_string() : index(none) {}
    constexpr basic_index_string(std::basic_string_view<char_t> view)
    {
        if (view.empty())
        {
            return;
        }

        auto hash = ELFHash(view.data(), view.length());

        auto map = __index_string_manager::GetIndexStringMap();

        auto it = map->find(hash);
        if (it == map->end())
        {
            index_string_block block;

            const size_t view_len = view.length();
            const size_t block_len = view_len + 1;

            block.bytes = new char_t[block_len];
            ::memcpy(block.bytes, view.data(), view_len);

            reinterpret_cast<char_t*>(block.bytes)[view_len] = 0;
            block.size = block_len * sizeof(char_t);

            map->emplace(hash, block);
        }
        else
        {
            const size_t block_len = it->second.size / sizeof(char_t);
            const size_t str_len = block_len - 1;

            bool collision;
            do
            {
                collision = false;
                if (str_len == view.length() && ::memcmp(it->second.bytes, view.data(), str_len * sizeof(char_t)) != 0)
                {
                    collision = true;
                    ++hash;
                }

            } while (collision);
        }
        index = hash;
    }

    static std::basic_string_view<char_t> get_string(index_t index)
    {
        auto map = __index_string_manager::GetIndexStringMap();
        auto it = map->find(index);
        if (it == map->end())
        {
            return {};
        }
        return std::basic_string_view(reinterpret_cast<char_t*>(it->second.bytes), it->second.size - 1);
    }

    std::basic_string<char_t> to_string() const
    {
        if (index == index_t{})
        {
            return {};
        }
        auto block = __index_string_manager::GetIndexStringMap()->find(index)->second;

        std::basic_string<char_t> str = reinterpret_cast<char_t*>(block.bytes);
        return str;
    }


    bool empty() const { return index == none; }

    bool operator==(const basic_index_string& str) const noexcept
    {
        return str.index == this->index;
    }

    basic_index_string& operator=(std::basic_string_view<char_t> view)
    {
        return *new(this)basic_index_string(view);
    }
    basic_index_string& operator=(basic_index_string str)
    {
        this->index = str.index;
        return *this;
    }

    index_t index{};
};

using index_string = basic_index_string<char>;

inline index_string operator ""_idxstr(const char* str, size_t len)
{
    return index_string{ str };
}
namespace std
{
    template<>
    struct hash<index_string>
    {
        size_t operator()(const index_string value) const noexcept
        {
            return value.index;
        }
    };
}