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

    using ObjectFlags = uint64_t;
    enum ObjectFlags_ : uint64_t
    {
        OF_NoFlag           = 0,
        OF_Persistent       = 1 << 0,
        OF_Instantiable     = 1 << 1,
        OF_Instance         = 1 << 2,
        OF_NoPack           = 1 << 3,
        OF_LifecycleManaged = 1 << 4,
    };

    enum class DependencyObjectState
    {
        Unload = 0x01,
        Reload = 0x02,
    };
    ENUM_CLASS_FLAGS(DependencyObjectState);


    class ObjectBase : public Object
    {
        friend class RuntimeObjectManager;
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ObjectBase, Object);
    public:
        ObjectBase();
        ~ObjectBase() noexcept override;
    public:
        [[always_inline]] ObjectHandle GetObjectHandle() const noexcept { return this->m_objectHandle; }
    public:
        void Construct(ObjectHandle handle = {});
        virtual void PostEditChange(FieldInfo* info);
    public:
        index_string GetIndexName() const noexcept { return m_name; }
        void         SetIndexName(index_string name) noexcept;
        string       GetName() const noexcept { return m_name.to_string(); }
        void         SetName(string_view name) noexcept;
        ObjectFlags  GetObjectFlags() const noexcept { return m_flags; }
        bool         HasObjectFlags(ObjectFlags flags) const noexcept { return m_flags & flags;}
        void         SetObjectFlags(uint16_t flags) noexcept { m_flags = flags; }
        bool         IsPersistentObject() const noexcept { return m_flags & OF_Persistent; }

        virtual void OnDependencyMessage(ObjectHandle inDependency, DependencyObjectState msg);

    protected:
        virtual void OnConstruct();
        virtual void OnDestroy();
    protected:
        void SendOuterDependencyMsg(DependencyObjectState msg) const;
    private:
        void Destroy();
        // base class 24
        index_string m_name;         // 8
        ObjectHandle m_objectHandle; // 16
    protected:
        ObjectFlags  m_flags{};      // 8
    public:
    };

    inline constexpr int kSizeObjectBase = sizeof(ObjectBase);

    struct ManagedPointer
    {
        friend class RuntimeObjectManager;
    private:
        ObjectBase* Pointer{};
        int Counter{};
    public:
        int RefCount() const noexcept { return Counter; }
        void Incref() { ++Counter; }
        int Decref() { --Counter; return Counter; }
        [[always_inline]] ObjectBase* Get() const
        {
            return Pointer;
        }
        void SetEmptyPtr()
        {
            Pointer = nullptr;
        }
    };

    class RuntimeObjectManager final
    {
    public:
        static ObjectBase* GetObject(const ObjectHandle& id) noexcept;
        static SPtr<ObjectBase> GetSharedObject(const ObjectHandle& id) noexcept;
        static SPtr<ManagedPointer> GetPointer(const ObjectHandle& id) noexcept;
        static SPtr<ManagedPointer> AddWaitPointer(const ObjectHandle& id);
        static bool IsValid(const ObjectHandle& id) noexcept;
        static void NewInstance(SPtr<ObjectBase>&& managedObj, const ObjectHandle& handle) noexcept;
        static bool DestroyObject(const ObjectHandle& id, bool isForce = false) noexcept;
        static void GetData(size_t* total, size_t* place, size_t* alive);
        static void Terminate();
        static void TickGCollect();
        static void ForEachObject(const std::function<void(ObjectHandle, ObjectBase*, int)>& func);

        //<id, type, is_create>
        static Action<ObjectHandle, Type*, bool> ObjectHook;
        static Action<ObjectBase*> OnPostEditChanged;

        static void AddDependList(ObjectHandle src, ObjectHandle dest);
        static void RemoveDependList(ObjectHandle src, ObjectHandle dest);
        static void NotifyDependObjects(ObjectHandle dest, DependencyObjectState id);
    };



    struct ObjectPtrBase
    {
        ObjectHandle Handle;
        SPtr<ManagedPointer> ManagedPtr;

        ObjectPtrBase() = default;
        ObjectPtrBase(const ObjectPtrBase&) = default;
        ObjectPtrBase(ObjectPtrBase&&) = default;
        ObjectPtrBase(const ObjectHandle& handle) : Handle(handle)
        {
            if (!handle.is_empty())
            {
                ManagedPtr = RuntimeObjectManager::GetPointer(handle);
                if (ManagedPtr == nullptr)
                {
                    ManagedPtr = RuntimeObjectManager::AddWaitPointer(handle);
                }
            }
        }
        ObjectPtrBase& operator=(const ObjectPtrBase&) = default;
        ObjectPtrBase& operator=(ObjectPtrBase&&) = default;

        [[always_inline]] const ObjectHandle& GetHandle() const
        {
            return Handle;
        }
        [[always_inline]] ObjectHandle& GetHandle()
        {
            return Handle;
        }
        [[always_inline]] ObjectBase* GetObjectPointer() const
        {
            if (ManagedPtr)
            {
                return ManagedPtr->Get();
            }
            return nullptr;
        }
    };

    class BoxingObjectPtrBase : public BoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxingObjectPtrBase, BoxingObject);
        CORELIB_IMPL_INTERFACES(IStringify)
    public:
        using unboxing_type = ObjectPtrBase;
        ObjectPtrBase get_unboxing_value() { return ptr; }
        BoxingObjectPtrBase() : CORELIB_INIT_INTERFACE(IStringify) {}
        explicit BoxingObjectPtrBase(const ObjectPtrBase& invalue) : ptr(invalue),
            CORELIB_INIT_INTERFACE(IStringify) {}

        void IStringify_Parse(const string& value) override
        {
            ptr = ObjectHandle::parse(value);
        }

        string IStringify_Stringify() override
        {
            return ptr.GetHandle().to_string();
        }

        [[always_inline]] ObjectHandle GetHandle() const noexcept
        {
            return ptr.Handle;
        }
        [[always_inline]] void SetHandle(const ObjectHandle& handle) noexcept
        {
            ptr.Handle = handle;
        }

        ObjectPtrBase ptr;
    };
}
CORELIB_DECL_BOXING(pulsar::ObjectPtrBase, pulsar::BoxingObjectPtrBase);

namespace pulsar
{

    template<typename T>
    concept baseof_objectbase = std::is_base_of<ObjectBase, T>::value;


    template<typename T>
    struct ObjectPtr : public ObjectPtrBase
    {
        using base = ObjectPtrBase;
        using element_type = T;

        using base::base;
        ObjectPtr(T* ptr) noexcept : base(ptr ? ptr->GetObjectHandle() : ObjectHandle{})
        {
        }

        ObjectPtr(const ObjectPtrBase& ptr)
        {
            Handle = ptr.Handle;
            ManagedPtr = ptr.ManagedPtr;
        }

        ObjectPtr(const SPtr<T>& ptr) : ObjectPtr(ptr.get())
        {
        }

        template<typename U> requires std::is_base_of_v<T, U>
        ObjectPtr(const ObjectPtr<U>& derived) noexcept
        {
            Handle = derived.Handle;
            ManagedPtr = derived.ManagedPtr;
        }

        ObjectPtr() = default;

        [[always_inline]] SPtr<T> GetShared() const noexcept
        {
            return sptr_cast<T>(RuntimeObjectManager::GetSharedObject(Handle));
        }
        [[always_inline]] T* GetPtr() const noexcept
        {
            return static_cast<T*>(GetObjectPointer());
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
        [[always_inline]] bool operator==(const ObjectPtrBase& r) const noexcept { return Handle == r.Handle; }
        [[always_inline]] bool operator==(std::nullptr_t) const noexcept { return !IsValid(); }
        template<typename U>
        bool operator==(const ObjectPtr<U>& r) const noexcept { return Handle == r.Handle; }

        [[always_inline]] bool IsValid() const noexcept
        {
            return GetObjectPointer() != nullptr;
        }
        [[always_inline]] explicit operator bool() const noexcept
        {
            return IsValid();
        }

        void Reset() noexcept
        {
            Handle = {};
            ManagedPtr = nullptr;
        }
    };

    template<typename T, typename U>
    ObjectPtr<T> ref_cast(ObjectPtr<U> o)
    {
        if (!o) return {};
        return ptr_cast<T>(o.GetPtr());
    }


    class RCPtrBase
    {
    public:
        ObjectHandle Handle;
        SPtr<ManagedPointer> ManagedPtr;
    protected:
        [[always_inline]] void Incref() const noexcept
        {
            if (GetPointer())
            {
                ManagedPtr->Incref();
            }
        }
        [[always_inline]] void Decref()
        {
            if (ManagedPtr->Decref() == 0)
            {
                if (ManagedPtr.use_count() == 2)
                    ManagedPtr.reset();
                RuntimeObjectManager::DestroyObject(Handle);
            }
        }
    public:
        [[always_inline]] ObjectHandle& GetHandle() noexcept
        {
            return Handle;
        }
        [[always_inline]] const ObjectHandle& GetHandle() const noexcept
        {
            return Handle;
        }
        [[always_inline]] ObjectBase* GetPointer() const noexcept
        {
            if (ManagedPtr)
            {
                return ManagedPtr->Get();
            }
            return nullptr;
        }
        RCPtrBase(const ObjectHandle& handle)
        {
            if (!handle.is_empty())
            {
                Handle = handle;
                ManagedPtr = RuntimeObjectManager::GetPointer(handle);
                if (!ManagedPtr)
                {
                    ManagedPtr = RuntimeObjectManager::AddWaitPointer(handle);
                }
                ManagedPtr->Incref();
            }
        }
        RCPtrBase() : Handle({}) {}
        RCPtrBase(const ObjectBase* ptr) : RCPtrBase(ptr ? ptr->GetObjectHandle() : ObjectHandle{})
        {
        }
        RCPtrBase(const RCPtrBase& ptr) noexcept : Handle(ptr.Handle), ManagedPtr(ptr.ManagedPtr)
        {
            if (ManagedPtr) ManagedPtr->Incref();
        }
        RCPtrBase(RCPtrBase&& ptr) noexcept : Handle(ptr.Handle), ManagedPtr(std::move(ptr.ManagedPtr))
        {
            ptr.ManagedPtr = nullptr;
        }
        RCPtrBase& operator=(const RCPtrBase& ptr) noexcept
        {
            if (this == &ptr) return *this;
            if (ManagedPtr) Decref();
            Handle = ptr.Handle;
            ManagedPtr = ptr.ManagedPtr;
            if (ManagedPtr) Incref();
            return *this;
        }
        RCPtrBase& operator=(RCPtrBase&& ptr) noexcept
        {
            if (ManagedPtr) Decref();
            Handle = ptr.Handle;
            ManagedPtr = std::move(ptr.ManagedPtr);
            return *this;
        }
        ~RCPtrBase() noexcept
        {
            if (ManagedPtr)
            {
                Decref();
            }
        }

        [[always_inline]] bool IsValid() const noexcept
        {
            if (ManagedPtr == nullptr)
            {
                return false;
            }
            return ManagedPtr->Get() != nullptr;
        }

        [[always_inline]] explicit operator bool() const noexcept
        {
            return IsValid();
        }

        bool operator==(const RCPtrBase& ptr) const noexcept
        {
            return Handle == ptr.Handle;
        }

        bool operator==(std::nullptr_t) const noexcept
        {
            return !IsValid();
        }

        void Reset()
        {
            Handle = {};
            ManagedPtr.reset();
        }
    };

    template <typename T>
    struct RCPtr : public RCPtrBase
    {
        using base = RCPtrBase;

        T* operator->() const
        {
            auto ptr = GetPtr();
            if (!ptr)
            {
                throw NullPointerException();
            }
            return ptr;
        }
        RCPtr() : base() {}
        RCPtr(const ObjectHandle& handle) : base(handle) {}
        RCPtr(const ObjectBase* ptr) : base(ptr ? ptr->GetObjectHandle() : ObjectHandle{}) {}
        RCPtr(const SPtr<T>& t) : base(t.get()) {}

        RCPtr(const RCPtrBase& ptr) : base(ptr) {}

        // RCPtr(const RCPtr& ptr) : base(ptr) {}
        // RCPtr(RCPtr&& ptr) noexcept : base(std::move(ptr))
        // {
        // }
        // RCPtr& operator=(const RCPtr& ptr) noexcept
        // {
        //     if (this == &ptr) return *this;
        //     if (ManagedPtr) Decref();
        //     Handle = ptr.Handle;
        //     ManagedPtr = ptr.ManagedPtr;
        //     if (ManagedPtr) Incref();
        //     return *this;
        // }
        // RCPtr& operator=(RCPtr&& ptr) noexcept
        // {
        //     if (ManagedPtr) Decref();
        //     Handle = ptr.Handle;
        //     ManagedPtr = std::move(ptr.ManagedPtr);
        //     ptr.Handle = {};
        //     ptr.ManagedPtr = nullptr;
        //     return *this;
        // }

        [[always_inline]] T* GetPtr() const noexcept
        {
            return static_cast<T*>(GetPointer());
        }

    };

    template<typename T, typename U>
    RCPtr<T> cref_cast(RCPtr<U> o)
    {
        if (!o) return {};
        return ptr_cast<T>(o.GetPtr());
    }

    class BoxingRCPtrBase : public BoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxingRCPtrBase, BoxingObject);
        CORELIB_IMPL_INTERFACES(IStringify)
    public:
        using unboxing_type = RCPtrBase;
        RCPtrBase get_unboxing_value() { return ptr; }
        BoxingRCPtrBase() : CORELIB_INIT_INTERFACE(IStringify), ptr({}) {}
        explicit BoxingRCPtrBase(const RCPtrBase& invalue) :
            CORELIB_INIT_INTERFACE(IStringify), ptr(invalue) {}

        void IStringify_Parse(const string& value) override
        {
            ptr = ObjectHandle::parse(value);
        }

        string IStringify_Stringify() override
        {
            return ptr.GetHandle().to_string();
        }

        [[always_inline]] ObjectHandle GetHandle() const noexcept
        {
            return ptr.GetHandle();
        }
        [[always_inline]] void SetHandle(const ObjectHandle& handle) noexcept
        {
            ptr = handle;
        }

        RCPtrBase ptr;
    };

    inline void DestroyObject(const ObjectPtrBase& object, bool isForce = false) noexcept
    {
        RuntimeObjectManager::DestroyObject(object.GetHandle(), isForce);
    }
    inline void DestroyObject(const RCPtrBase& object, bool isForce = false) noexcept
    {
        RuntimeObjectManager::DestroyObject(object.GetHandle(), isForce);
    }

    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, ObjectPtrBase& obj);
    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, RCPtrBase& obj);

}
CORELIB_DECL_BOXING(pulsar::RCPtrBase, pulsar::BoxingRCPtrBase);

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

    template<typename T>
    struct type_redirect<pulsar::RCPtr<T>>
    {
        using type = pulsar::RCPtrBase;
    };

    template<typename T>
    struct type_wrapper<pulsar::RCPtr<T>>
    {
        using type = T;
    };;
}

namespace pulsar
{
    DECL_PTR(ObjectBase);
}
namespace std
{

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

    template<>
    struct hash<pulsar::RCPtrBase>
    {
        size_t operator()(const pulsar::RCPtrBase& ptr) const noexcept
        {
            return std::hash<pulsar::ObjectHandle>()(ptr.GetHandle());
        }
    };

    template<typename T>
    struct hash<pulsar::RCPtr<T>>
    {
        size_t operator()(const pulsar::RCPtr<T>& ptr) const noexcept
        {
            return std::hash<pulsar::ObjectHandle>()(ptr.GetHandle());
        }
    };
}