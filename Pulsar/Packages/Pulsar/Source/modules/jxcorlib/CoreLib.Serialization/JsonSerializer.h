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

    class ISerializeObjectHook
    {
    public:
        // 返回 true 表示已处理，返回 false 则序列化器使用默认解析路径
        virtual bool IStringify_Parse(Object* object, const string& value) = 0;
    };

    class JsonSerializer
    {
    public:
        static string Serialize(Object* obj, const JsonSerializerSettings& settings);
    public:
        static SPtr<Object> Deserialize(
            const string& jstr,
            Type* type,
            Object_sp defaultObject = nullptr,
            ISerializeObjectHook* hook = nullptr);

        template<typename T, typename O = Object>
        static SPtr<T> Deserialize(const string& str)
        {
            return jxcorlib::sptr_cast<T>( Deserialize(str, cltypeof<T>() ) );
        }
    };

}
