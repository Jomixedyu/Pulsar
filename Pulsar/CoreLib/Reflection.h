/*
* @Moudule     : Reflection
* @Date        : 2021/05/23
* @Author      : JomiXedYu
* @Description : This file is part of jxcorlib : https://github.com/JomiXedYu/jxcorlib
* @StdRequired : c++20
*/

#pragma once

#include "UString.h"
#include "Object.h"
#include "Type.h"
#include "Assembly.h"
#include "BasicTypes.h"
#include "Enum.h"

#include <functional>
#include <any>
#include <memory>


//支持 基础值类型 系统内指针（包括智能指针）类型
#define CORELIB_REFL_DECL_FIELD(NAME) \
    static inline struct __corelib_refl_##NAME \
    { \
        template<typename T> using _Detected = decltype(std::declval<T&>().NAME); \
        __corelib_refl_##NAME() \
        { \
            using FieldType = decltype(std::declval<ThisClass&>().NAME); \
            using DecayType = fulldecay<FieldType>::type; \
            using CleanType = remove_shared_ptr<DecayType>::type; \
            using ClType = get_boxing_type<remove_shared_ptr<std::remove_cv<FieldType>::type>::type>::type; \
            ReflectionBuilder::CreateFieldInfo<ThisClass, FieldType>( \
                #NAME, false, jxcorlib::is_detected<_Detected, ThisClass>::value, \
                [](Object* p) -> sptr<Object> { \
                    auto rawptr = (ThisClass*)p; \
                    return get_object_pointer<CleanType>::get(rawptr->NAME); \
                }, \
                [](Object* p, sptr<Object> value) { \
                    auto rawptr = (ThisClass*)p; \
                    object_assign<CleanType>::assign(&rawptr->NAME, value); \
                }); \
        } \
    } __corelib_refl_##NAME##_;

#define CORELIB_REFL_DECL_FUNC(NAME) \
    static inline struct __corelib_refl_##NAME \
    { \
        __corelib_refl_##NAME() \
        { \
            array_list<ParameterInfo*> infos; \
            ReflectionBuilder::CreateMethodParameterInfos(NAME, &infos); \
        } \
    } __corelib_refl_##NAME##_;

namespace jxcorlib
{
    class TypeInfo : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::TypeInfo, Object);
    public:
        TypeInfo() {}
        TypeInfo(const TypeInfo&) = delete;
        TypeInfo(TypeInfo&&) = delete;
    };

    class MemberInfo : public TypeInfo
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::MemberInfo, TypeInfo);
    protected:
        string name_;
        bool is_static_;
        bool is_public_;
    public:
        const string& get_name() const { return this->name_; }
        bool is_static() const { return this->is_static_; }
        bool is_public() const { return this->is_public_; }
    public:
        MemberInfo(const string& name, bool is_static, bool is_public);
        MemberInfo(const MemberInfo& right) = delete;
        MemberInfo(MemberInfo&& right) = delete;
    };

    class FieldInfo final : public MemberInfo
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::FieldInfo, MemberInfo);
    public:
        struct FieldTypeInfo
        {
            bool is_raw_pointer;
            bool is_shared_pointer;
            bool is_const;
        };
        using GetterFunction = std::function<sptr<Object>(Object* instance)>;
        using SetterFunction = std::function<void(Object* instance, sptr<Object> value)>;
    protected:
        FieldTypeInfo info_;
        Type* field_type_;
        GetterFunction getter_;
        SetterFunction setter_;
    public:
        Type* get_field_type() const { return this->field_type_; }
        bool is_raw_pointer() const { return this->info_.is_raw_pointer; }
        bool is_shared_pointer() const { return this->info_.is_shared_pointer; }
        bool is_pointer() const { return is_raw_pointer() || is_shared_pointer(); }
        bool is_const() const { return this->info_.is_const; }

    public:
        FieldInfo(
            const string& name, bool is_static, bool is_public,
            FieldTypeInfo info, Type* field_type,
            const GetterFunction& getter, const SetterFunction& setter);

        FieldInfo(const FieldInfo& right) = delete;
        FieldInfo(FieldInfo&& right) = delete;
    public:
        void SetValue(Object* instance, sptr<Object> value);
        sptr<Object> GetValue(Object* instance) const;

    };
    class ParameterInfo : public TypeInfo
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::ParameterInfo, TypeInfo);
    protected:
        Type* param_type_;
        bool is_pointer_;
        bool is_const_;
        bool is_ref_;
        bool is_rref_;
    public:
        Type* get_param_type() const { return this->param_type_; }
        bool is_pointer() const { return this->is_pointer_; }
        bool is_const() const { return this->is_const_; }
        bool is_reference() const { return this->is_ref_; }
        bool is_rreference() const { return this->is_rref_; }
    public:
        ParameterInfo(const ParameterInfo&) = delete;
        ParameterInfo(ParameterInfo&&) = delete;
        ParameterInfo(
            Type* param_type,
            bool is_pointer,
            bool is_const,
            bool is_ref,
            bool is_rref)
            : param_type_(param_type),
            is_pointer_(is_pointer),
            is_const_(is_const),
            is_ref_(is_ref),
            is_rref_(is_rref) {}
    };

    //TODO
    class MethodInfo final : public MemberInfo
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::MethodInfo, MemberInfo);
    protected:
        array_list<ParameterInfo*> param_types_;
        ParameterInfo* ret_type_;
        bool is_abstract_;
    public:
        const std::vector<ParameterInfo*>& get_parameter_infos() const noexcept { return this->param_types_; }
        ParameterInfo* get_return_type() const noexcept { return this->ret_type_; }
        bool is_abstract() const { return this->is_abstract_; }
    public:
        MethodInfo(
            const string& name, bool is_static, bool is_public,
            ParameterInfo* ret_type, array_list<ParameterInfo*>&& params_infos, bool is_abstract);
        MethodInfo(const MethodInfo& right) = delete;
        MethodInfo(MethodInfo&& right) = delete;
    public:
        Object_sp Invoke(Object* instance, array_list<Object_sp>&& params) const;
    };


    class ReflectionBuilder
    {
    private:

    public:
        template<typename T, typename TField>
        static inline void CreateFieldInfo(
            const string& name,
            bool is_static, bool is_public,
            const FieldInfo::GetterFunction& getter,
            const FieldInfo::SetterFunction& setter
        )
        {
            FieldInfo::FieldTypeInfo info;
            info.is_raw_pointer = std::is_pointer<TField>::value;
            info.is_const = std::is_const<TField>::value;
            info.is_shared_pointer = is_shared_ptr<TField>::value;

            using ClType = get_boxing_type<typename fulldecay<TField>::type>::type;
            Type* field_type = cltypeof<ClType>();

            cltypeof<T>()->_AddMemberInfo(new FieldInfo{ name, is_static, is_public, info, field_type, getter, setter });
        }

    private:

        static void _GetParameters(array_list<ParameterInfo*>* out) {}

        template<typename T, typename... P>
        static void _GetParameters(array_list<ParameterInfo*>* out, T* t, P*... params)
        {
            using Box = typename get_boxing_type<T>::type;
            out->push_back(new ParameterInfo(cltypeof<Box>(),
                std::is_pointer_v<Box>,
                std::is_const_v<Box>,
                std::is_reference_v<Box>,
                std::is_rvalue_reference_v<Box>));
            _GetParameters(out, params...);
        }

        template<typename T>
        static constexpr T* GenNullptr() { return (T*)nullptr; }
    public:

        template<typename R, typename... P>
        static void CreateMethodParameterInfos(R(*p)(P...), array_list<ParameterInfo*>* out)
        {
            _GetParameters(out, GenNullptr<P>()...);
        }


        static void CreateMethodInfo(Type* type, const string& name, bool is_static, bool is_public, array_list<ParameterInfo*>&& info)
        {
            //todo: return value
            type->_AddMemberInfo(new MethodInfo(name, is_static, is_public, nullptr, std::move(info), false));
        }
    };
}
