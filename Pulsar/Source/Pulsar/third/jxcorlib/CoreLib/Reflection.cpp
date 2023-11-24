#include "Reflection.h"
#include "Attribute.h"

namespace jxcorlib
{
    sptr<Attribute> TypeInfo::GetAttribute(Type* type)
    {
        for (auto& attr : this->m_attributes)
        {
            if (type->IsInstanceOfType(attr.get()))
            {
                return attr;
            }
        }
        return nullptr;
    }

    array_list<sptr<Attribute>> TypeInfo::GetAttributes(Type* type)
    {
        array_list<sptr<Attribute>> ret;
        for (auto& attr : this->m_attributes)
        {
            if (type->IsInstanceOfType(attr.get()))
            {
                ret.push_back(attr);
            }
        }
        return ret;
    }

    bool TypeInfo::IsDefinedAttribute(Type* type)
    {
        return GetAttribute(type) != nullptr;
    }

    FieldInfo::FieldInfo(
        const string& name, bool is_public,
        FieldTypeInfo info, Type* field_type,
        Type* typeWrapper,
        GetterFunction&& getter,
        SetterFunction&& setter
        )
        : base(name, is_public),
          m_info(info), m_fieldType(field_type), m_typeWrapper(typeWrapper),
          m_getter(std::move(getter)), m_setter(std::move(setter))
    {

    }


    void FieldInfo::SetValue(Object* instance, sptr<Object> value)
    {
        this->m_setter(instance, std::move(value));
    }

    sptr<Object> FieldInfo::GetValue(Object* instance) const
    {
        return this->m_getter(instance);
    }

    MemberInfo::MemberInfo(const string& name, bool is_public)
        : m_name(name), m_isPublic(is_public)
    {
    }

    MethodInfo::MethodInfo(
        const string& name,
        bool isPublic,
        bool isStatic,
        ParameterInfo* retType,
        array_list<ParameterInfo*>&& paramsInfos,
        bool isAbstract,
        std::unique_ptr<MethodDescription>&& delegate
        )
        : base(name, isPublic),
          m_isStatic(isStatic),
          m_retType(retType), m_paramTypes(paramsInfos), m_isAbstract(isAbstract), m_delegate(std::move(delegate))
    {
    }

    Object_sp MethodInfo::Invoke(Object_rsp instance, array_list<Object_sp>&& params) const
    {
        return this->m_delegate->CreateDelegate(instance)->DynamicInvoke(params);
    }


}
