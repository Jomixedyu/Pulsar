/*
* @Moudule     : Converter
* @Date        : 2021/07/14
* @Author      : JomiXedYu
* @Description : This file is part of jxcorlib : https://github.com/JomiXedYu/jxcorlib
* @StdRequired : c++11
*/

#pragma once

#include <string>
#include <cstdint>
#include <algorithm>

namespace jxcvt
{
    using std::to_string;

    int32_t to_int32(std::string_view str);
    float to_float32(std::string_view str);
    double to_double64(std::string_view str);
    bool to_bool(std::string_view str);

    inline int32_t to_int32(std::string_view str)
    {
        return std::atoi(str.data());
    }
    inline float to_float32(std::string_view str)
    {
        return (float)std::atof(str.data());
    }
    inline double to_double64(std::string_view str)
    {
        return std::atof(str.data());
    }
    inline bool to_bool(std::string_view str)
    {
        if (str.size() > 5 || str.size() < 4)
        {
            return false;
        }
        std::string nstr(str);
        std::transform(nstr.begin(), nstr.end(), nstr.begin(), ::tolower);
        return nstr == "true";
    }


    template<typename T>
    concept is_iteratable = requires { typename T::iterator; };

    template<is_iteratable T>
    inline std::string to_string(const T& arr)
    {
        std::string s;
        s.append("[");
        auto last = arr.end(); 
        --last;

        for (auto it = arr.begin(); it != arr.end(); it++)
        {
            s.append(to_string(*it));
            if (it != last)
            {
                s.append(", ");
            }
        }
        s.append("]");
        return s;
    }
}


namespace jxcorlib
{
    using namespace jxcvt;
}
