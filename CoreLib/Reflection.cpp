#include "Reflection.h"


namespace jxcorlib
{
    FieldInfo::FieldInfo(
        const string& name, bool is_static, bool is_public,
        FieldTypeInfo info, Type* field_type,
        const GetterFunction& getter,
        const SetterFunction& setter
    )
        : base(name, is_static, is_public),
        info_(info), field_type_(field_type), getter_(getter), setter_(setter)
    {

    }



    void FieldInfo::SetValue(Object* instance, sptr<Object> value)
    {
        this->setter_(instance, value);
    }

    sptr<Object> FieldInfo::GetValue(Object* instance) const
    {
        return this->getter_(instance);
    }

    MemberInfo::MemberInfo(const string& name, bool is_static, bool is_public)
        : name_(name), is_static_(is_static), is_public_(is_public)
    {
    }

    MethodInfo::MethodInfo(
        const string& name, bool is_static, bool is_public,
        ParameterInfo* ret_type, array_list<ParameterInfo*>&& params_infos, bool is_abstract
    ) : base(name, is_static, is_public),
        ret_type_(ret_type), param_types_(params_infos), is_abstract_(is_abstract)
    {
    }

    Object_sp MethodInfo::Invoke(Object* instance, array_list<Object_sp>&& params) const
    {
        
        return nullptr;
    }


}