#pragma once

#include <CoreLib/List.h>
#include <cstdint>
#include <string>
#include <memory>
#include <cassert>
#include <CoreLib/CommonException.h>
#include <CoreLib/Events.hpp>
#include <CoreLib/Guid.h>
#include <Pulsar/EngineMath.h>
#include <unordered_map>
#include <unordered_set>
#include <CoreLib/IndexString.h>
#include <CoreLib/sser.hpp>
#include "Assembly.h"

#define WITH_EDITOR

#define DECL_PTR(Class) \
CORELIB_DECL_SHORTSPTR(Class); \
using Class##_ref = ::pulsar::ObjectPtr<Class>;

#define THIS_REF ObjectPtr<ThisClass>(GetObjectHandle())

#define assert_err(expr, msg)
#define assert_warn(expr, msg)

namespace pulsar
{
    class EngineException : public ExceptionBase
    {
    public:
        virtual const char* name() const override { return "EngineException"; }
        using ExceptionBase::ExceptionBase;
    };

    using ObjectHandle = guid_t;

    template<typename K, typename V>
    using hash_map = std::unordered_map<K, V>;

    template<typename T>
    using hash_set = std::unordered_set<T>;

    using ObjectFlags = uint16_t;
    enum ObjectFlags_ : uint16_t
    {
        OF_NoFlag           = 0,
        OF_Persistent       = 1 << 0,
        OF_Instantiable     = 1 << 1,
        OF_Instance         = 1 << 2,
        OF_NoPack           = 1 << 3,
        OF_LifecycleManaged = 1 << 4,
    };

    class ObjectBase : public Object
    {
        friend class RuntimeObjectWrapper;
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ObjectBase, Object);
    public:
        ObjectBase();
        ~ObjectBase() noexcept override;
    public:
        ObjectHandle GetObjectHandle() const noexcept { return this->m_objectHandle; }
    public:
        void Construct(ObjectHandle handle = {});
        virtual void PostEditChange(FieldInfo* info);
    public:
        index_string GetIndexName() const noexcept { return m_name; }
        void         SetIndexName(index_string name) noexcept;
        string       GetName() const noexcept { return m_name.to_string(); }
        void         SetName(string_view name) noexcept;
        uint16_t     GetObjectFlags() const noexcept { return m_flags; }
        bool         HasObjectFlags(ObjectFlags flags) const noexcept { return m_flags & flags;}
        void         SetObjectFlags(uint16_t flags) noexcept { m_flags = flags; }
        bool         IsPersistentObject() const noexcept { return m_flags & OF_Persistent; }
    protected:
        virtual void OnConstruct();
        virtual void OnDestroy();
    private:
        void Destroy();
        // base 24
        index_string m_name;         // 8
        ObjectHandle m_objectHandle; // 16
    protected:
        ObjectFlags  m_flags{};      // 16

        #ifdef WITH_EDITOR
        string m_objectName;
        #endif
    public:
    };

    inline constexpr int kSizeObjectBase = sizeof(ObjectBase);

    class RuntimeObjectWrapper final
    {
    public:
        static ObjectBase* GetObject(ObjectHandle id) noexcept;
        static sptr<ObjectBase> GetSharedObject(ObjectHandle id) noexcept;
        static bool IsValid(ObjectHandle id) noexcept;
        static void NewInstance(sptr<ObjectBase>&& managedObj, ObjectHandle handle) noexcept;
        static bool DestroyObject(ObjectHandle id, bool isForce = false) noexcept;
        static void Terminate();

        static void ForEachObject(const std::function<void(ObjectHandle, ObjectBase*)>& func);

        //<id, type, is_create>
        static Action<ObjectHandle, Type*, bool> ObjectHook;
        static Action<ObjectBase*> OnPostEditChanged;
    };

    struct ObjectPtrBase
    {
        ObjectHandle handle{};

        bool operator==(const ObjectPtrBase& b) const
        {
            return handle == b.handle;
        }
        ObjectHandle GetHandle() const
        {
            return handle;
        }
        template<typename T>
        T* GetTPtr() const
        {
            return static_cast<T*>(RuntimeObjectWrapper::GetObject(handle));
        }
    };

    class BoxingObjectPtrBase : public BoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxingObjectPtrBase, BoxingObject);
        CORELIB_IMPL_INTERFACES(IStringify)
    public:
        using unboxing_type = ObjectPtrBase;
        ObjectPtrBase get_unboxing_value() { return { handle }; }
        BoxingObjectPtrBase() : CORELIB_INIT_INTERFACE(IStringify) {}
        explicit BoxingObjectPtrBase(ObjectPtrBase invalue) : handle(invalue.handle),
            CORELIB_INIT_INTERFACE(IStringify) {}

        void IStringify_Parse(const string& value) override
        {
            handle = ObjectHandle::parse(value);
        }

        string IStringify_Stringify() override
        {
            return handle.to_string();
        }

        ObjectHandle handle;
    };
}
CORELIB_DECL_BOXING(pulsar::ObjectPtrBase, pulsar::BoxingObjectPtrBase);

namespace pulsar
{

    template<typename T>
    concept baseof_objectbase = std::is_base_of<ObjectBase, T>::value;


    inline bool IsValid(const ObjectPtrBase& object) noexcept
    {
        return RuntimeObjectWrapper::IsValid(object.handle);
    }

    inline void DestroyObject(const ObjectPtrBase& object, bool isForce = false) noexcept
    {
        RuntimeObjectWrapper::DestroyObject(object.handle, isForce);
    }

    template<typename T>
    struct ObjectPtr : public ObjectPtrBase
    {
        using element_type = T;
        T* Ptr;

        ObjectPtr(ObjectHandle inHandle) noexcept
        {
            handle = inHandle;
            Ptr = GetPtr();
        }
        ObjectPtr(ObjectPtrBase objectptr) noexcept
        {
            handle = objectptr.handle;
            Ptr = GetPtr();
            assert(Ptr->GetType()->IsSubclassOf(cltypeof<T>()));
        }
        ObjectPtr(T* ptr) noexcept
        {
            if (Ptr = ptr)
            {
                handle = ptr->GetObjectHandle();
            }
        }


        template<typename U> requires std::is_base_of_v<T, U>
        ObjectPtr(const ObjectPtr<U>& other) noexcept
        {
            handle = other.handle;
            Ptr = GetPtr();
        }
        ObjectPtr() noexcept : Ptr(nullptr) {}
        ObjectPtr(std::nullptr_t) noexcept : Ptr(nullptr)  {}
        ObjectPtr(const sptr<T>& object) noexcept
        {
            if (object != nullptr)
            {
                handle = object->GetObjectHandle();
            }
            Ptr = GetPtr();
        }

        sptr<T> GetShared() const noexcept
        {
            return sptr_cast<T>(RuntimeObjectWrapper::GetSharedObject(handle));
        }
        T* GetPtr() const noexcept
        {
            return GetTPtr<T>();
        }
        T* operator->() const noexcept(false)
        {
            auto ptr = GetPtr();
            if(ptr == nullptr)
            {
                throw NullPointerException();
            }
            return ptr;
        }
        bool operator==(const ObjectPtrBase& r) const { return handle == r.handle; }
        bool operator==(std::nullptr_t) const { return !IsValid(); }

        bool operator!=(const ObjectPtrBase& r) const { return handle != r.handle; }
        template<typename U>
        bool operator==(const ObjectPtr<U>& r) const { return handle == r.handle; }

        bool IsValid() const noexcept
        {
            return RuntimeObjectWrapper::IsValid(handle);
        }
        operator bool() const noexcept
        {
            return IsValid();
        }
        void Reset() noexcept
        {
            handle = {};
        }
    };

    template<typename T, typename U>
    ObjectPtr<T> ref_cast(ObjectPtr<U> o)
    {
        if (!o) return {};
        return ptr_cast<T>(o.GetPtr());
    }

    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, ObjectPtrBase& obj);

    class NotifyFieldValueChangedAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::NotifyFieldValueChangedAttribute, Attribute);
    public:
        NotifyFieldValueChangedAttribute(const char* name)
            : m_name(name)
        {
        }
    public:
        const char* GetPostMethodName() const
        {
            return m_name;
        }
    private:
        const char* m_name;
    };
}
namespace jxcorlib
{
    template<typename T>
    struct type_redirect<pulsar::ObjectPtr<T>>
    {
        using type = pulsar::ObjectPtrBase;
    };

    template<typename T>
    struct type_wrapper<pulsar::ObjectPtr<T>>
    {
        using type = T;
    };
}

namespace pulsar
{
    DECL_PTR(ObjectBase);
}
namespace std
{
    template<>
    struct hash<pulsar::ObjectHandle>
    {
        size_t operator()(const pulsar::ObjectHandle& handle) const noexcept
        {
            return *reinterpret_cast<const uint64_t*>(&handle) ^ *((reinterpret_cast<const uint64_t*>(&handle) + 1));
        }
    };

    template<>
    struct hash<pulsar::ObjectPtrBase>
    {
        size_t operator()(const pulsar::ObjectPtrBase& ptr) const noexcept
        {
            return std::hash<pulsar::ObjectHandle>()(ptr.GetHandle());
        }
    };

    template<typename T>
    struct hash<pulsar::ObjectPtr<T>>
    {
        size_t operator()(const pulsar::ObjectPtr<T>& ptr) const noexcept
        {
            return std::hash<pulsar::ObjectHandle>()(ptr.GetHandle());
        }
    };
}