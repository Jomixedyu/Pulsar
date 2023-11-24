#pragma once

#include "Assembly.h"
#include <CoreLib/Attribute.h>

namespace jxcorlib::ser
{
    struct JsonSerializerSettings
    {
        int32_t IndentSpace = -1;
        bool    StringEnum = false;
        bool    SaveObjectType = false;
    };

    namespace util
    {
        string GetSerializableTypeName(Type* type);
        Type* GetSerializableType(string_view name);
    }

    class SerializableArrayAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_JxSerialization, jxcorlib::ser::SerializableArrayAttribute, Attribute);
    public:
        SerializableArrayAttribute(Type* itemType) : m_itemType(itemType) {}

        Type* GetItemType() const { return m_itemType; }
    private:
        Type* m_itemType;
    };

    class JsonSerializer
    {
    public:
        static string Serialize(Object* obj, const JsonSerializerSettings& settings);
    public:
        static sptr<Object> Deserialize(const string& jstr, Type* type);
        template<typename T, typename O = Object>
        static sptr<T> Deserialize(const string& str)
        {
            return jxcorlib::sptr_cast<T>( Deserialize(str, cltypeof<T>() ) );
        }
    };

}
