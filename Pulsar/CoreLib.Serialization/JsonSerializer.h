#pragma once

#include "Assembly.h"

namespace jxcorlib::ser
{
    struct JsonSerializerSettings
    {
        int32_t indent_space = -1;
        bool string_enum = false;
    };

    class JsonSerializer
    {
    public:
        static string Serialize(Object* obj, const JsonSerializerSettings& settings);
    public:
        static sptr<Object> Deserialize(const string& jstr, Type* type, sptr<Object> default_v = nullptr);
        template<typename T, typename O = Object>
        static sptr<T> Deserialize(const string& str, sptr<O> default_v = nullptr)
        {
            return jxcorlib::sptr_cast<T>(
                Deserialize(str, cltypeof<T>(), jxcorlib::sptr_cast<Object>(default_v) ) );
        }
    };

}
