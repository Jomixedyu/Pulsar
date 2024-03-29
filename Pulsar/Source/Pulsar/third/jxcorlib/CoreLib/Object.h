/*
* @Moudule     : Object
* @Date        : 2021/04/17
* @Author      : JomiXedYu
* @Description : This file is part of jxcorlib : https://github.com/JomiXedYu/jxcorlib
* @StdRequired : c++20
*/

#pragma once

#include <vector>
#include <type_traits>
#include <memory>
#include "UString.h"

#define CORELIB_DECL_ASSEMBLY(NAME) \
    inline struct __corelib_AssemblyClass_##NAME : public ::jxcorlib::AssemblyTypeObject \
    {  const char* name() { return #NAME; } } AssemblyObject_##NAME;

#define CORELIB_DECL_SHORTSPTR(CLASS) static_assert(sizeof(CLASS)); \
    using CLASS##_sp = ::jxcorlib::SPtr<class CLASS>; \
    using CLASS##_rsp = const ::jxcorlib::SPtr<class CLASS>&; \
    using CLASS##_wp = ::jxcorlib::WPtr<class CLASS>;

#define CORELIB_DECL_TEMP_SHORTSPTR(NAME) \
    template<typename T> \
    using NAME##_sp = ::jxcorlib::SPtr<NAME<T>>; \
    template<typename T> \
    using NAME##_rsp = const NAME##_sp<T>&; \
    template<typename T> \
    using NAME##_wp = ::jxcorlib::WPtr<NAME<T>>;

#ifdef WIN32
    #if defined(JXCORELIB_BUILD_SHARED) && defined(JXCORELIB_EXPORT_API)
        #define JXCORELIB_API __declspec(dllexport)
    #elif defined(JXCORELIB_BUILD_SHARED) && !defined(JXCORELIB_EXPORT_API)
        #define JXCORELIB_API __declspec(dllimport)
    #else
        #define JXCORELIB_API
    #endif
#else
    #define JXCORELIB_API
#endif

#ifndef always_inline
    #if   defined(_MSC_VER)
        #define always_inline msvc::forceinline
    #elif defined(__GNUC__)
        #define always_inline gnu::always_inline
    #elif defined(__clang__)
        #define always_inline clang::always_inline
    #else
        #define always_inline always_inline
    #endif
#endif

namespace jxcorlib
{
    class Type;
    class Assembly;
    class Object;

    struct AssemblyTypeObject
    {
        virtual const char* name() = 0;
    };

    CORELIB_DECL_ASSEMBLY(jxcorlib);

    template<typename T>
    using SPtr = std::shared_ptr<T>;

    template<typename Tout, typename Tin>
    SPtr<Tout> sptr_static_cast(const SPtr<Tin>& other)
    {
        return std::static_pointer_cast<Tout, Tin>(other);
    }
    template<typename Tout, typename Tin>
    SPtr<Tout> sptr_static_cast(SPtr<Tin>&& other)
    {
        return std::static_pointer_cast<Tout, Tin>(std::move(other));
    }

    template<typename Tout, typename Tin>
    SPtr<Tout> sptr_cast(const SPtr<Tin>& other)
    {
        if (other == nullptr) return nullptr;
        if (Tout::StaticType()->IsInstanceOfType(other.get()))
            return std::static_pointer_cast<Tout, Tin>(other);
        return nullptr;
    }
    template<typename Tout, typename Tin>
    SPtr<Tout> sptr_cast(SPtr<Tin>&& other)
    {
        if (other == nullptr) return nullptr;
        if (Tout::StaticType()->IsInstanceOfType(other.get()))
            return std::static_pointer_cast<Tout, Tin>(std::move(other));
        return nullptr;
    }

    template<typename T>
    SPtr<T> mksptr(T* t) { return SPtr<T>(t); }

    template<typename T>
    using WPtr = std::weak_ptr<T>;

    template<typename T>
    WPtr<T> mkwptr(const SPtr<T>& ptr) { return WPtr<T>(ptr); }

    template<typename T, typename = void>
    struct is_shared_ptr
    {
        constexpr inline static bool value = false;
    };
    template<typename T>
    struct is_shared_ptr<T, std::void_t<typename T::element_type>>
    {
        constexpr inline static bool value = std::is_same<T, std::shared_ptr<typename T::element_type>>::value;
    };

    template<typename T, typename = void>
    struct remove_shared_ptr
    {
        using type = T;
    };
    template<typename T>
    struct remove_shared_ptr<T, std::void_t<typename T::element_type>>
    {
        using type = typename T::element_type;
    };

    template <typename T>
    void init_sptr_member(SPtr<T>& ptr)
    {
        ptr = mksptr(new T);
    }

    class Object : public std::enable_shared_from_this<Object>
    {
    public:
        static Type* StaticType();
    private:
        static inline struct _ObjectInit {
            _ObjectInit() { Object::StaticType(); }
        } _object_init_;
    private:
    public:
        virtual Type* GetType() const;
        friend class Type;
    public:
        Object() {}
        virtual ~Object();
    public:
        virtual string ToString() const;
        virtual bool Equals(Object* object) const;
        //hidden in derived
        bool Equals(const SPtr<Object>& object) const { return this->Equals(object.get()); }
        bool EqualsSptr(const SPtr<Object>& object) const { return this->Equals(object.get()); }

        template<typename T, typename U>
        static bool StaticEquals(const T& a, const U& b)
        {
            if (a == b) return true;
            if (a == nullptr || b == nullptr) return false;
            return a->Equals(b);
        }
    };

    constexpr int kSizeObject = sizeof(Object);

    CORELIB_DECL_SHORTSPTR(Object);


    template<typename Tout>
    inline Tout* ptr_cast(Object* o)
    {
        if (!o) return nullptr;
        if (Tout::StaticType()->IsInstanceOfType(o))
            return static_cast<Tout*>(o);
        return nullptr;
    }

}
namespace std
{
    string to_string(jxcorlib::Object* obj);
    string to_string(const jxcorlib::SPtr<jxcorlib::Object>& obj);
}
