/*
* @Moudule     : Type
* @Date        : 2021/05/15
* @Author      : JomiXedYu
* @Description : This file is part of jxcorlib : https://github.com/JomiXedYu/jxcorlib
* @StdRequired : c++20
*/

#pragma once
#include <vector>
#include <map>
#include <any>
#include <type_traits>
#include <memory>
#include <functional>

#include "UString.h"
#include "Object.h"
#include "EnumUtil.h"
#include "IInterface.h"


//声明CoreLib元数据
#define CORELIB_DEF_TYPE(ASSEMBLY, NAME, BASE) \
public: static inline ::jxcorlib::Type* StaticType() \
    { \
        static_assert(!std::is_same_v<NAME, BASE>, "base class error"); \
        static_assert(std::is_base_of<BASE, NAME>::value, "The base class does not match"); \
        static ::jxcorlib::Type* type = nullptr; \
        if (type == nullptr) \
        { \
            auto dynptr = ::jxcorlib::TypeTraits::get_dyninstpointer<ThisClass>::get_value(); \
            if (dynptr == nullptr) \
            { \
                dynptr = ::jxcorlib::TypeTraits::get_zeroparam_object<ThisClass>::get(); \
            } \
            ::jxcorlib::Assembly* assm = ::jxcorlib::AssemblyManager::BuildAssemblyByName(ASSEMBLY.name()); \
            type = new ::jxcorlib::Type(dynptr, assm, ::jxcorlib::cltypeof<BASE>(), #NAME, typeid(NAME), sizeof(NAME)); \
            assm->RegisterType(type); \
        } \
        return type; \
    } \
private: \
    using base      = BASE; \
    using ThisClass = NAME; \
    friend class ::jxcorlib::Type; \
    friend class ::jxcorlib::TypeTraits; \
public: \
    inline virtual ::jxcorlib::Type* GetType() const { return StaticType(); } \
private: \
    static inline struct __corelib_type { \
        __corelib_type() { NAME::StaticType(); } \
    } __corelib_type_init_; \
    ::jxcorlib::SPtr<ThisClass>         self()  { return ::jxcorlib::sptr_cast<ThisClass>(shared_from_this()); } \
    ::jxcorlib::SPtr<const ThisClass>   self()  const { return ::jxcorlib::sptr_cast<const ThisClass>(shared_from_this()); } \
    ::jxcorlib::WPtr<ThisClass>         self_weak() { return ::jxcorlib::WPtr<ThisClass>(self()); } \
    ::jxcorlib::WPtr<const ThisClass>   self_weak() const { return ::jxcorlib::WPtr<const ThisClass>(self()); }


#define CORELIB_DEF_ENUMTYPE(ASSEMBLY, NAME, BASE) \
public: static inline ::jxcorlib::Type* StaticType() \
    { \
        static_assert(!std::is_same_v<NAME, BASE>, "base class error"); \
        static_assert(std::is_base_of<BASE, NAME>::value, "The base class does not match"); \
        static ::jxcorlib::Type* type = nullptr; \
        if (type == nullptr) \
        { \
            auto dynptr = ::jxcorlib::TypeTraits::get_dyninstpointer<ThisClass>::get_value(); \
            if (dynptr == nullptr) \
            { \
                dynptr = ::jxcorlib::TypeTraits::get_zeroparam_object<ThisClass>::get(); \
            } \
            ::jxcorlib::Assembly* assm = ::jxcorlib::AssemblyManager::BuildAssemblyByName(ASSEMBLY.name()); \
            type = new ::jxcorlib::Type(dynptr, assm, ::jxcorlib::cltypeof<BASE>(), #NAME, typeid(NAME), sizeof(NAME)); \
            assm->RegisterType(type); \
            ::jxcorlib::TypeBuilder::RegisterEnum(type, &ThisClass::StaticGetDefinitions); \
        } \
        return type; \
    } \
private: \
    using base = BASE; \
    using ThisClass = NAME; \
    friend class ::jxcorlib::Type; \
    friend class ::jxcorlib::TypeTraits; \
public: \
    inline virtual ::jxcorlib::Type* GetType() const { \
        return StaticType(); \
    } \
private: \
    static inline struct __corelib_type { \
        __corelib_type() { NAME::StaticType(); } \
    } __corelib_type_init_;


#define CORELIB_DEF_INTERFACE(ASSEMBLY, NAME, BASE) \
public: static inline ::jxcorlib::Type* StaticType() \
    { \
        static_assert(!std::is_same_v<NAME, BASE>, "base class error"); \
        static_assert(std::is_base_of<BASE, NAME>::value, "The base class does not match"); \
        static ::jxcorlib::Type* type = nullptr; \
        if (type == nullptr) \
        { \
            ::jxcorlib::Assembly* assm = ::jxcorlib::AssemblyManager::BuildAssemblyByName(ASSEMBLY.name()); \
            type = new ::jxcorlib::Type(nullptr, assm, ::jxcorlib::cltypeof<BASE>(), #NAME, typeid(NAME), sizeof(NAME)); \
            assm->RegisterType(type); \
        } \
        return type; \
    } \
private: \
    using base = BASE; \
    using ThisClass = NAME; \
    friend class ::jxcorlib::Type; \
    friend class ::jxcorlib::TypeTraits; \
public: \
    using base::base; \
private: \
    static inline struct __corelib_type { \
        __corelib_type() { NAME::StaticType(); } \
    } __corelib_type_init_; \
public:

#define CORELIB_INIT_INTERFACE(INTERFACE) INTERFACE(StaticType())

#define CORELIB_DEF_TINTERFACE(ASSEMBLY, NAME, BASE, ...) \
public: static inline ::jxcorlib::Type* StaticType() \
    { \
        static_assert(!std::is_same_v<NAME, BASE>, "base class error"); \
        static_assert(std::is_base_of<BASE, NAME<__VA_ARGS__>>::value, "The base class does not match"); \
        static ::jxcorlib::Type* type = nullptr; \
        if (type == nullptr) \
        { \
            ::jxcorlib::Assembly* assm = ::jxcorlib::AssemblyManager::BuildAssemblyByName(ASSEMBLY.name()); \
            type = new ::jxcorlib::Type(nullptr, assm, ::jxcorlib::cltypeof<BASE>(), #NAME, typeid(NAME<__VA_ARGS__>), sizeof(NAME<__VA_ARGS__>)); \
            assm->RegisterType(type); \
        } \
        return type; \
    } \
private: \
    using base = BASE; \
    using ThisClass = NAME<__VA_ARGS__>; \
    friend class ::jxcorlib::Type; \
    friend class ::jxcorlib::TypeTraits; \
private: \
    static inline struct __corelib_type { \
        __corelib_type() { NAME<__VA_ARGS__>::StaticType(); } \
    } __corelib_type_init_; \
public:

//声明CoreLib模板元数据
#define CORELIB_DEF_TEMPLATE_TYPE(ASSEMBLY, NAME, BASE, ...) \
public: static inline ::jxcorlib::Type* StaticType() \
{ \
    static ::jxcorlib::Type* type = nullptr; \
    if (type == nullptr) \
    { \
        auto dynptr = ::jxcorlib::TypeTraits::get_dyninstpointer<ThisClass>::get_value(); \
        if (dynptr == nullptr) \
        { \
            dynptr = ::jxcorlib::TypeTraits::get_zeroparam_object<ThisClass>::get(); \
        } \
        using TemplateType = ::jxcorlib::TemplateTypePair<__VA_ARGS__>; \
        ::jxcorlib::Assembly* assm = ::jxcorlib::AssemblyManager::BuildAssemblyByName(ASSEMBLY.name()); \
        type = new ::jxcorlib::Type(dynptr, assm, ::jxcorlib::cltypeof<BASE>(), ::jxcorlib::StringUtil::Concat(#NAME, "<", typeid(TemplateType).name(), ">"), typeid(NAME<__VA_ARGS__>), sizeof(NAME<__VA_ARGS__>), true); \
        assm->RegisterType(type); \
    } \
    return type; \
} \
private: \
    using base = BASE; \
    using ThisClass = NAME<__VA_ARGS__>; \
    friend class ::jxcorlib::Type; \
    friend class ::jxcorlib::TypeTraits; \
    public: \
        inline virtual ::jxcorlib::Type* GetType() const override { \
            return StaticType(); \
        } \
    private: \
        static inline struct __corelib_type { \
            __corelib_type() { NAME<__VA_ARGS__>::StaticType(); } \
        } __corelib_type_init_; \
        ::jxcorlib::SPtr<ThisClass> self() { return ::jxcorlib::sptr_cast<ThisClass>(shared_from_this()); } \
        ::jxcorlib::SPtr<const ThisClass> self() const { return ::jxcorlib::sptr_cast<const ThisClass>(shared_from_this()); } \
        ::jxcorlib::WPtr<ThisClass> self_weak() { return ::jxcorlib::WPtr<ThisClass>(self()); } \
        ::jxcorlib::WPtr<const ThisClass> self_weak() const { return ::jxcorlib::WPtr<const ThisClass>(self()); }


#define CORELIB_IMPL_INTERFACES(...) \
    static inline struct __corelib_interface_list { \
        __corelib_interface_list() { ::jxcorlib::TypeBuilder::RegisterInterfaces<ThisClass, __VA_ARGS__>(StaticType()); } \
    } __corelib_interface_list_init_;


//反射工厂创建函数声明
#define CORELIB_DECL_DYNCINST() \
    static Object* DynCreateInstance(const ParameterPackage& params)

#define CORELIB_DECL_BOXING(ValueType, BoxingType) \
namespace jxcorlib \
{ \
    template<> struct get_boxing_type<ValueType> { using type = BoxingType; }; \
}


namespace jxcorlib
{
    struct ParameterPackage;

    class Type;
    class MemberInfo;
    class FieldInfo;
    class MethodInfo;
    class ReflectionBuilder;
    class Attribute;

    enum class TypeBinding : int32_t
    {
        None = 0,
        NonPublic = 1,
    };
    ENUM_CLASS_FLAGS(TypeBinding);

    template<typename T>
    using array_list = std::vector<T>;

    template<typename K, typename V>
    using map = std::map<K, V>;

    class IInterface
    {
    private:
        friend class Type;
        friend class TypeTraits;
        using ThisClass = IInterface;
        static inline struct __corelib_type {
            __corelib_type() { IInterface::StaticType(); }
        } __corelib_type_init_;
        Type* owner_type_;

    public:
        virtual ~IInterface() = default;
        IInterface(Type* owner) : owner_type_(owner) {}
        Type* GetType() const { return this->owner_type_; }
        static Type* StaticType();
    };
    CORELIB_DECL_SHORTSPTR(IInterface);

    template<typename T>
    concept baseof_object = std::is_base_of_v<Object, T>;

    template<typename T>
    concept cltype_concept =
        std::is_base_of<Object, typename remove_shared_ptr<typename std::remove_pointer<T>::type>::type>::value ||
        std::is_base_of<IInterface, typename remove_shared_ptr<typename std::remove_pointer<T>::type>::type>::value;

    template<typename T>
    concept cltype_sptr_concept =
        std::is_base_of_v<Object, typename remove_shared_ptr<T>::type> && is_shared_ptr<T>::value;

    template<typename T>
    concept newable_concept = requires { new T; };

    template<typename T>
    concept non_newable_concept = !requires { new T; };

    class Type : public Object
    {
    private:
        friend class Assembly;
        friend class TypeBuilder;
        friend class IInterface;
        friend class Enum;
        using EnumDatas      = std::vector<std::pair<string, uint32_t>>;
    public:
        using CreateInstFunc = Object * (*)(const ParameterPackage&);
        using SharedInterfaceGetter = std::function<IInterface_sp(Object_rsp)>;
        using InterfaceGetter = std::function<IInterface* (Object*)>;
        using EnumGetter = const EnumDatas* (*)();
    private:
        string                 m_name;
        string                 m_shortName;
        int32_t                m_structureSize;
        Type*                  m_base;
        CreateInstFunc         m_createInstanceFunc;
        const std::type_info&  m_typeinfo;
        array_list<Type*>*     m_templateTypes;
        Assembly*              m_assembly;
        EnumGetter             m_enumGetter;
        bool                   m_isInterface;
        bool                   m_isGeneric;

        array_list<SPtr<Attribute>>    m_attributes;
        std::map<string, SPtr<MemberInfo>>  m_memberInfos;
        array_list<std::tuple<Type*, InterfaceGetter, SharedInterfaceGetter>> m_interfaces;

    private:
        Type(const Type& r) = delete;
        Type(Type&& r) = delete;

        static inline struct _TypeInit {
            _TypeInit() {
                Type::StaticType();
            }
        } _type_init_;
    public:
        static Type*  StaticType();
        virtual Type* GetType() const { return StaticType(); }
        Assembly*     GetAssembly() const { return this->m_assembly; }
    public:
        virtual int32_t         GetStructureSize() const { return this->m_structureSize; }
        const string&           GetName() const { return this->m_name; }
        const string            GetShortName() const { return this->m_shortName; }
        Type*                   GetBase() const { return this->m_base; }
        const std::type_info&   GetTypeInfo() const { return this->m_typeinfo; }
        array_list<Type*>       GetInterfaces() const;

        bool IsPrimitiveType() const;
        bool IsBoxingType() const;
        bool IsInterface() const { return this->m_isInterface; }
        bool IsGeneric() const { return this->m_isGeneric; }
        bool IsEnum() const { return this->m_enumGetter; }
        bool IsImplementedInterface(Type* type);
        bool IsInstanceOfType(const Object* object) const;
        bool IsSharedInstanceOfType(const SPtr<Object>& ptr) const;
        bool IsSubclassOf(const Type* type) const;

        template <typename T>
        SPtr<T> GetAttribute(bool inherit = true) { return sptr_static_cast<T>(GetAttribute(T::StaticType(), inherit)); }

        SPtr<Attribute>             GetAttribute(Type* type, bool inherit = true);
        array_list<SPtr<Attribute>> GetAttributes(Type* type, bool inherit = true);
        bool                        IsDefinedAttribute(Type* type, bool inherit = true);

        Object*      CreateInstance(const ParameterPackage& v);
        SPtr<Object> CreateSharedInstance(const ParameterPackage& v);
    public:
        virtual string ToString() const override;
    public:
        Type(
            CreateInstFunc dyncreate,
            Assembly* assembly,
            Type* base,
            const string& name,
            const std::type_info& info,
            int32_t structure_size,
            bool isGeneric = false);

        template<cltype_concept T>
        static inline Type* Typeof()
        {
            return T::StaticType();
        }
        /* Reflection */
    public:
        friend class MemberInfo;
        friend class FieldInfo;
        friend class MethodInfo;
        friend class ReflectionBuilder;
    public:
        MemberInfo*             GetMemberInfo(const string& name);
        void                    GetMemberInfos(array_list<MemberInfo*>& out, TypeBinding attr = TypeBinding::None);
        array_list<MemberInfo*> GetMemberInfos(TypeBinding attr = TypeBinding::None);

        FieldInfo*              GetFieldInfo(const string& name);
        void                    GetFieldInfos(array_list<FieldInfo*>& out, TypeBinding attr = TypeBinding::None);
        array_list<FieldInfo*>  GetFieldInfos(TypeBinding attr = TypeBinding::None);

        MethodInfo*             GetMethodInfo(const string& name);
        void                    GetMethodInfos(array_list<MethodInfo*>& out, TypeBinding attr = TypeBinding::None);
        array_list<MethodInfo*> GetMethodInfos(TypeBinding attr = TypeBinding::None);

        IInterface_sp           GetSharedInterface(Object_rsp instance, Type* type);
        IInterface*             GetInterface(Object* instance, Type* type);


        struct EnumAccessor
        {
            std::map<string, uint32_t> s2i;
            std::map<uint32_t, string> i2s;
            bool GetName(uint32_t enumValue, string* outName) const
            {
                auto it = i2s.find(enumValue);
                if (it == i2s.end())
                {
                    return false;
                }
                if (outName) *outName = it->second;
                return true;
            }
            bool GetIndex(const string& name, uint32_t* outEnumValue) const
            {
                auto it = s2i.find(name);
                if (it == s2i.end())
                {
                    return false;
                }
                if (outEnumValue) *outEnumValue = it->second;
                return true;
            }
        };

        const EnumAccessor*    GetEnumAccessors();
    private:
        void _AddMemberInfo(MemberInfo* info);
        std::unique_ptr<EnumAccessor> m_enumAccessor;
    public:

    };

    template<baseof_object TOut, baseof_object TIn> requires(std::is_base_of_v<TIn, TOut>)
    inline TOut* object_cast(TIn* obj)
    {
        if (obj == nullptr) return nullptr;
        if (cltypeof<TOut>()->IsInstanceOfType(obj))
        {
            return static_cast<TOut*>(obj);
        }
        return nullptr;
    }

    template<typename T> struct fulldecay { using type = T; };
    template<typename T> struct fulldecay<const T> : fulldecay<T> { };
    template<typename T> struct fulldecay<T&> : fulldecay<T> { };
    template<typename T> struct fulldecay<T*> : fulldecay<T> { };
    template<typename T> struct fulldecay<volatile T> : fulldecay<T> { };
    template<typename T> struct fulldecay<T[]> : fulldecay<T> { };
    template<typename T, int I>  struct fulldecay<T[I]> : fulldecay<T> { };

    template<cltype_concept T>
    inline Type* cltypeof()
    {
        return Type::Typeof<T>();
    }

    //inline bool istype(Object* obj, Type* type)
    //{
    //    return type->IsInstanceOfType(obj);
    //}

    struct ParameterPackage
    {
    private:
        std::vector<std::any> data;
    public:
        ParameterPackage() {}
        ParameterPackage(std::initializer_list<std::any> lst) : data(lst.begin(), lst.end()) { }
        template<typename T>
        inline void Add(const T& v) {
            data.push_back(std::any(v));
        }
        template<typename T>
        inline T Get(const int& index) const {
            return std::any_cast<T>(data[index]);
        }
        inline size_t Count() const { return data.size(); }
        inline bool IsEmpty() const { return this->data.empty(); }
    private:
        template<int I>  bool _Check() const { return true; }

        template<int I, typename T, typename... TArgs>
        bool _Check() const {
            return (this->data[I].type() == typeid(T)) & _Check<I + 1, TArgs...>();
        }
    public:
        template<typename... TArgs>
        inline bool Check() const {
            if (this->data.size() != sizeof...(TArgs)) {
                return false;
            }
            return _Check<0, TArgs...>();
        }
    };



    class TypeTraits final
    {
    public:
        template<typename T, typename = void>
        struct is_zeroparam_ctor : std::false_type {};

        template<typename T>
        struct is_zeroparam_ctor<T, std::void_t<decltype(T())>> : std::true_type {};

        template<typename T, bool HasZeroParamCtor = is_zeroparam_ctor<T>::value>
        struct get_zeroparam_object {};

        template<typename T>
        struct get_zeroparam_object<T, true>
        {
            using value_type = Object * (*)(const ParameterPackage&);
            static value_type get()
            {
                return [](const ParameterPackage&) -> Object* {
                    return new T;
                };
            }
        };
        template<typename T>
        struct get_zeroparam_object<T, false>
        {
            using value_type = Object * (*)(const ParameterPackage&);
            static value_type get()
            {
                return nullptr;
            }
        };

        template<typename T>
        struct has_dyninstc
        {
        private:
            template<typename U, Object* (*)(const ParameterPackage&) = &U::DynCreateInstance>
            static constexpr bool check(U*) { return true; }
            static constexpr bool check(...) { return false; }
        public:
            static constexpr bool value = check(static_cast<T*>(0));
        };

        template<typename T, bool Test = has_dyninstc<T>::value>
        struct get_dyninstpointer {};

        template<typename T>
        struct get_dyninstpointer<T, true>
        {
            using value_type = Object * (*)(const ParameterPackage&);
            //member method
            static value_type get_value()
            {
                return &T::DynCreateInstance;
            }
        };

        template<typename T>
        struct get_dyninstpointer<T, false>
        {
            using value_type = Object * (*)(const ParameterPackage&);
            //nullptr
            static value_type get_value()
            {
                return nullptr;
            }
        };


    };

    class StdAny;


    template<typename T, bool b = ::jxcorlib::cltype_concept<T>>
    struct get_boxing_type
    {};

    template<typename T>
    struct get_boxing_type<T, true>
    {
        using type = typename ::jxcorlib::remove_shared_ptr<T>::type;
    };

    template<typename T>
    struct get_boxing_type<T, false>
    {
        static_assert(true, "no boxing type!");
    };

    template<typename T>
    using get_boxing_type_t = typename get_boxing_type<T>::type;

    template<typename T>
    SPtr<T> interface_sptr_cast(Object_rsp obj)
    {
        if (obj == nullptr) return nullptr;
        return sptr_static_cast<T>(obj->GetType()->GetSharedInterface(obj, cltypeof<T>()));
    }

    template<typename T>
    T* interface_cast(Object* obj)
    {
        if (obj == nullptr) return nullptr;
        return (T*)obj->GetType()->GetInterface(obj, cltypeof<T>());
    }

    template<typename T, bool iscl = cltype_concept<T>>
    struct get_object_pointer
    {
    };

    template<typename T>
    struct get_object_pointer<T, true>
    {
        static SPtr<Object> get(const SPtr<T>& t)
        {
            return t;
        }
    };

    template<typename T>
    struct get_object_pointer<T, false>
    {
        using ClType = get_boxing_type<T>::type;
        static SPtr<Object> get(const T& t)
        {
            return mksptr(new ClType(t));
        }
    };

    template<typename T>
    struct type_redirect
    {
        using type = T;
    };

    template<typename T>
    using type_redirect_t = typename type_redirect<T>::type;

    template<typename T>
    struct type_wrapper
    {
        using type = T;
    };

    template<typename T, bool iscl = cltype_concept<T>>
    struct object_assign
    {};

    template<typename T>
    struct object_assign<T, true>
    {
        static void assign(SPtr<T>* target, SPtr<Object>& value)
        {
            *target = std::static_pointer_cast<T>(value);
        }
    };
    template<typename T>
    struct object_assign<T, false>
    {
        using ClType = get_boxing_type<T>::type;
        static void assign(T* target, SPtr<Object>& value)
        {
            auto ptr = static_cast<ClType*>(value.get());
            *target = ptr->get_unboxing_value();
        }
    };


    template <typename, template <typename...> class Op, typename... T>
    struct is_detected_impl : std::false_type {};
    template <template <typename...> class Op, typename... T>
    struct is_detected_impl<std::void_t<Op<T...>>, Op, T...> : std::true_type {};

    template <template <typename...> class Op, typename... T>
    using is_detected = is_detected_impl<void, Op, T...>;



    class TypeBuilder
    {
    public:
        static void RegisterInterface(Type* self, Type* interface_type, const Type::InterfaceGetter& cast, const Type::SharedInterfaceGetter& scast)
        {
            self->m_interfaces.push_back({ interface_type, cast, scast });
            self->m_isInterface = true;
        }
    private:
        template<typename T>
        static void _RegisterInterfaces(Type* self) { }

        template<typename T, typename TInterface, typename... TInterfaces>
        static void _RegisterInterfaces(Type* self)
        {
            RegisterInterface(self, cltypeof<TInterface>(),
                [](Object* obj) -> IInterface* {
                    return (IInterface*)(T*)obj;
                },
                [](Object_rsp obj) -> IInterface_sp {
                    auto sobj = sptr_cast<T>(obj);
                    return sptr_static_cast<IInterface>(sobj);
                }
                );
            _RegisterInterfaces<T, TInterfaces...>(self);
        }
    public:

        template<typename T, typename... TInterfaces>
        static void RegisterInterfaces(Type* self)
        {
            _RegisterInterfaces<T, TInterfaces...>(self);
        }
    public:
        static void RegisterEnum(Type* type, Type::EnumGetter enum_getter)
        {
            type->m_enumGetter = enum_getter;
        }

        template<typename... TArgs>
        static void RegisterAttributes(Type* type, TArgs&&... args)
        {
            for (auto& i : std::initializer_list{ args... })
            {
                type->m_attributes.push_back(mksptr(i));
            }
        }
    };

}
