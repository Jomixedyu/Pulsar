#pragma once
#include "Object.h"
#include "Type.h"
#include "Assembly.h"

namespace jxcorlib
{
    class BoxingObject : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::BoxingObject, Object);

    };

    class IStringify : public IInterface
    {
        CORELIB_DEF_INTERFACE(AssemblyObject_jxcorlib, jxcorlib::IStringify, IInterface);

        virtual void IStringify_Parse(const string& value) = 0;
        virtual string IStringify_Stringify() = 0;
    };

    class PrimitiveObject : public BoxingObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::PrimitiveObject, BoxingObject);
    public:
    };

#define __CORELIB_DEF_BASE_TYPE(Class, DataType) \
    class Class final : public PrimitiveObject \
    { \
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::Class, PrimitiveObject);\
        CORELIB_DECL_DYNCINST() { \
            if (params.Count() != 1 || !params.Check<DataType>()) \
            { \
                return new Class{ DataType{0} }; \
            } \
            return new Class{ params.Get<DataType>(0) }; \
        } \
    public: \
        using unboxing_type = DataType; \
        DataType value; \
        DataType get_unboxing_value() { return this->value; } \
        Class(DataType value) : value(value) { } \
        operator DataType() { return value; } \
        virtual string ToString() const override { return std::to_string(value); } \
    }; \
    CORELIB_DECL_SHORTSPTR(Class); \
    inline bool operator==(const Class& l, const DataType& r) { return l.value == r; } \
    inline bool operator==(const DataType& l, const Class& r) { return l == r.value; } \
    inline bool operator!=(const Class& l, const DataType& r) { return l.value != r; } \
    inline bool operator!=(const DataType& l, const Class& r) { return l != r.value; } \
    template<> struct get_boxing_type<DataType> { using type = Class; };
    
    __CORELIB_DEF_BASE_TYPE(CharObject, char);
    __CORELIB_DEF_BASE_TYPE(Integer8, int8_t);
    __CORELIB_DEF_BASE_TYPE(UInteger8, uint8_t);
    __CORELIB_DEF_BASE_TYPE(Integer16, int16_t);
    __CORELIB_DEF_BASE_TYPE(UInteger16, uint16_t);
    __CORELIB_DEF_BASE_TYPE(Integer32, int32_t);
    __CORELIB_DEF_BASE_TYPE(UInteger32, uint32_t);
    __CORELIB_DEF_BASE_TYPE(Integer64, int64_t);
    __CORELIB_DEF_BASE_TYPE(UInteger64, uint64_t);
    __CORELIB_DEF_BASE_TYPE(Single32, float);
    __CORELIB_DEF_BASE_TYPE(Double64, double);
    __CORELIB_DEF_BASE_TYPE(Boolean, bool);

    class String final : public PrimitiveObject, public string
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::String, PrimitiveObject);
        CORELIB_DECL_DYNCINST()
        {
            if (params.Count() == 0)
            {
                return new String;
            }
            if (params.Count() != 1 || !params.Check<const char*>())
            {
                return nullptr;
            }
            return new String{ params.Get<const char*>(0) };
        }
    public:
        using string::basic_string;
        String(const string& right) : string::basic_string(right) {  }
        String() : string::basic_string() {  }

        virtual string ToString() const override { return *this; }
        string get_unboxing_value() { return *this; }
        static sptr<String> FromString(string_view str)
        {
            return mksptr(new String(str));
        }
    };
    CORELIB_DECL_SHORTSPTR(String);
    template<> struct get_boxing_type<string> { using type = String; };

    class StdAny : public Object, public std::any
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::StdAny, Object);
        CORELIB_DECL_DYNCINST() {
            return nullptr;
        }
    public:

    private:
        template<typename TValue>
        static bool _AnyCast(const std::any& any, TValue* t) { return false; }

        template<typename TValue, typename TCastable1, typename... TCastable>
        static bool _AnyCast(const std::any& any, TValue* t)
        {
            auto name = any.type().name();
            auto cname = typeid(TCastable1).name();
            if (any.type() == typeid(TCastable1))
            {
                *t = std::any_cast<TCastable1>(any);
                return true;
            }
            else
            {
                return _AnyCast<TValue, TCastable...>(any, t);
            }
            return false;
        }

    public:
        template<typename TValue, typename... TCastable>
        static bool AnyCast(const std::any& any, TValue* t)
        {
            return _AnyCast<TValue, TCastable...>(any, t);
        }
    };
    template<> struct get_boxing_type<std::any> { using type = StdAny; };


    class PrimitiveObjectUtil
    {
    private:
        template<typename TValue, typename TType>
        static inline bool _Assign(TValue* t, Object* value)
        {
            if (value->GetType() == cltypeof<TType>())
            {
                *t = static_cast<TType*>(value)->get_unboxing_value();
                return true;
            }
            return false;
        }

    public:
        template<typename T>
        static bool Assign(T* t, Object* value)
        {
            return
                _Assign<T, String>(t, value) ||
                _Assign<T, Integer32>(t, value) ||
                _Assign<T, UInteger32>(t, value) ||
                _Assign<T, Single32>(t, value) ||
                _Assign<T, Double64>(t, value) ||
                _Assign<T, Boolean>(t, value) ||
                _Assign<T, Integer8>(t, value) ||
                _Assign<T, UInteger8>(t, value) ||
                _Assign<T, Integer16>(t, value) ||
                _Assign<T, UInteger16>(t, value) ||
                _Assign<T, Integer64>(t, value) ||
                _Assign<T, UInteger64>(t, value);
        }
    };


    template<typename... T>
    struct TemplateTypePair
    {
        static std::vector<Type*>* GetTemplateTypes()
        {
            auto* vec = new std::vector<Type*>;
            (vec->push_back(cltypeof<typename get_boxing_type<T>::type>()), ...);
            return vec;
        }
    };


    class IList : public IInterface
    {
        CORELIB_DEF_INTERFACE(AssemblyObject_jxcorlib, jxcorlib::IList, IInterface);

        virtual void Add(Object_rsp value) = 0;
        virtual Object_sp At(int32_t index) const = 0;
        virtual void SetAt(int32_t index, Object_rsp value) = 0;
        virtual void Clear() = 0;
        virtual void RemoveAt(int32_t index) = 0;
        virtual int32_t IndexOf(Object_rsp value) const = 0;
        virtual bool Contains(Object_rsp value) = 0;
        virtual int32_t GetCount() const = 0;
        virtual Type* GetIListElementType() const = 0;
    };
    CORELIB_DECL_SHORTSPTR(IList);

    struct BoxUtil
    {
        template<typename T>
        static inline sptr<Object> Box(const T& value)
        {
            if constexpr (cltype_sptr_concept<T>)
            {
                return value;
            }
            else
            {
                return mksptr((Object*)new get_boxing_type_t<T>(value));
            }
        }
    };


    struct UnboxUtil
    {
        template<typename T>
        static inline T Unbox(const sptr<Object>& value)
        {
            return static_cast<get_boxing_type_t<T>*>(value.get())->get_unboxing_value();
        }
		
        template<typename T>
        static inline auto TryUnbox(const sptr<Object>& value)
        {
            if constexpr (std::is_base_of_v<BoxingObject, T>)
            {
                return static_cast<get_boxing_type_t<T>*>(value.get())->get_unboxing_value();
            }
            else
            {
                return sptr_cast<T>(value);
            }
        }
    };

    template<typename T>
    sptr<typename get_boxing_type<T>::type> mkbox(T value)
    {
        return mksptr(new get_boxing_type_t<T>(value));
    }


    namespace ObjectUtil
    {
        void DeepCopyObject(const sptr<Object>& from, sptr<Object>& to);
        sptr<Object> DeepCopyObject(const sptr<Object>& from);
    }

}