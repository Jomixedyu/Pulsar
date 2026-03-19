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


#define assert_err(expr, msg)
#define assert_warn(expr, msg)

namespace pulsar
{

    struct ObjectHandle
    {
        uint64_t value = 0;
        ObjectHandle() = default;


        static ObjectHandle parse(const std::string& str)
        {
            ObjectHandle h;
            h.value = std::stoull(str);
            return h;
        }
        std::string to_string() const
        {
            return std::to_string(value);
        }
        bool operator==(const ObjectHandle& other) const
        {
            return value == other.value;
        }
        bool is_empty() const
        {
            return value == 0ull;
        }
        operator bool() const
        {
            return value != 0ull;
        }
    };

    class BoxingObjectHandle : public BoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxingObjectHandle, BoxingObject);
        CORELIB_IMPL_INTERFACES(IStringify)
    public:
        using unboxing_type = ObjectHandle;
        ObjectHandle get_unboxing_value() { return ptr; }
        BoxingObjectHandle() : CORELIB_INIT_INTERFACE(IStringify) {}
        explicit BoxingObjectHandle(const ObjectHandle& invalue) : ptr(invalue),
                                                                     CORELIB_INIT_INTERFACE(IStringify) {}

        void IStringify_Parse(const string& value) override
        {
            ptr = ObjectHandle::parse(value);
        }

        string IStringify_Stringify() override
        {
            return ptr.to_string();
        }

        [[always_inline]] ObjectHandle GetHandle() const noexcept
        {
            return ptr;
        }
        [[always_inline]] void SetHandle(const ObjectHandle& handle) noexcept
        {
            ptr = handle;
        }

        ObjectHandle ptr;
    };
    //    using ObjectHandle = guid_t;
}
CORELIB_DECL_BOXING(pulsar::ObjectHandle, pulsar::BoxingObjectHandle);


namespace std
{
    template<>
    struct hash<pulsar::ObjectHandle>
    {
        size_t operator()(const pulsar::ObjectHandle& ptr) const noexcept
        {
            return std::hash<uint64_t>()(ptr.value);
        }
    };
}

namespace pulsar
{
    class EngineException : public ExceptionBase
    {
    public:
        virtual const char* name() const override { return "EngineException"; }
        using ExceptionBase::ExceptionBase;
    };



    template<typename K, typename V>
    using hash_map = std::unordered_map<K, V>;

    template<typename T>
    using hash_set = std::unordered_set<T>;

    using ObjectFlags = uint64_t;
    enum ObjectFlags_ : uint64_t
    {
        OF_NoFlag           = 0,
        OF_Transient        = 1 << 1,
        OF_Instantiable     = 1 << 2,
        OF_NoPack           = 1 << 3,
        OF_LifecycleManaged = 1 << 4,
    };

    enum class DependencyObjectState
    {
        Unload = 0x01,
        Modified = 0x02,
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
        void Construct();
        virtual void PostEditChange(FieldInfo* info);
    public:
        index_string GetIndexName() const noexcept { return m_name; }
        void         SetIndexName(index_string name) noexcept;
        string       GetName() const noexcept { return m_name.to_string(); }
        void         SetName(string_view name) noexcept;
        ObjectFlags  GetObjectFlags() const noexcept { return m_flags; }
        bool         HasObjectFlags(ObjectFlags flags) const noexcept { return m_flags & flags;}
        void         SetObjectFlags(ObjectFlags flags) noexcept { m_flags = flags; }
        bool         IsTransientObject() const noexcept { return m_flags & OF_Transient; }

        virtual void OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg);

        virtual void GetSubscribeObserverHandles(array_list<ObjectHandle>& out) {}
        void RebuildObserver();
    protected:
        virtual void OnConstruct();
        virtual void OnDestroy();

    protected:
        void SendOuterDependencyMsg(DependencyObjectState msg) const;

    protected:
        virtual void Destroy();
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

        ObjectBase* Pointer{};
        int Counter{};

        int RefCount() const noexcept { return Counter; }
        void Incref() { ++Counter; }
        int Decref()
        {
            --Counter;
            return Counter;
        }
        [[always_inline]] ObjectBase* Get() const
        {
            return Pointer;
        }
        void SetEmptyPtr()
        {
            Pointer = nullptr;
        }
    };

    struct RuntimeObjectInfo
    {
        ObjectHandle Handle;
        ObjectBase* Pointer{};
        int ManagedCounter{};
    };

    class RuntimeObjectManager final
    {
    public:
        static ObjectBase* GetObject(const ObjectHandle& id) noexcept;
        static SPtr<ObjectBase> GetSharedObject(const ObjectHandle& id) noexcept;
        static SPtr<ManagedPointer> GetPointer(const ObjectHandle& id) noexcept;
        static bool IsValid(const ObjectHandle& id) noexcept;
        static void NewInstance(SPtr<ObjectBase>&& managedObj) noexcept;
        static bool DestroyObject(const ObjectHandle& id, bool isForce = false) noexcept;
        static void GetData(size_t* total, size_t* place, size_t* alive);
        static void Terminate();
        static void TickGCollect();
        static void ForEachObject(const std::function<void(const RuntimeObjectInfo&)>& func);

        //<id, type, is_create>
        static Action<ObjectHandle, Type*, bool> ObjectHook;
        static Action<ObjectBase*> OnPostEditChanged;

        static void NotifyDependencySource(ObjectHandle dest, DependencyObjectState id);
        static void RebuildMessageBox(ObjectBase* obj);
    };

    class InlineObjectAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InlineObjectAttribute, Attribute);
    };

    struct ObjectPtrBase
    {
        SPtr<ManagedPointer> ManagedPtr;

        ObjectPtrBase() = default;
        ObjectPtrBase(const ObjectPtrBase&) = default;
        ObjectPtrBase(ObjectPtrBase&&) = default;
        ObjectPtrBase(const ObjectHandle& handle)
        {
            ManagedPtr = RuntimeObjectManager::GetPointer(handle);
        }
        explicit ObjectPtrBase(std::nullptr_t) {}

        ObjectPtrBase& operator=(const ObjectPtrBase&) = default;
        ObjectPtrBase& operator=(ObjectPtrBase&&) = default;

        ObjectBase* operator->() const
        {
            return GetObjectPointer();
        }

        [[always_inline]] ObjectHandle GetHandle() const
        {
            if (auto ptr = GetObjectPointer())
            {
                return ptr->GetObjectHandle();
            }
            return {};
        }
        [[always_inline]] ObjectBase* GetObjectPointer() const
        {
            if (ManagedPtr)
            {
                return ManagedPtr->Get();
            }
            return nullptr;
        }
        bool IsEmpty() const
        {
            return ManagedPtr == nullptr;
        }
        bool IsValid() const
        {
            return GetObjectPointer() != nullptr;
        }
        void Reset()
        {
            ManagedPtr = nullptr;
        }
        operator bool() const
        {
            return IsValid();
        }
    };

    class BoxingObjectPtrBase : public PointerBoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxingObjectPtrBase, PointerBoxingObject);
        CORELIB_IMPL_INTERFACES(IStringify)
    public:
        using unboxing_type = ObjectPtrBase;
        ObjectPtrBase get_unboxing_value() { return ptr; }
        BoxingObjectPtrBase() : CORELIB_INIT_INTERFACE(IStringify) {}
        explicit BoxingObjectPtrBase(const ObjectPtrBase& invalue) : ptr(invalue),
            CORELIB_INIT_INTERFACE(IStringify) {}

        Object* GetPointer() const override { return ptr.GetObjectPointer(); }

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
    private:
        friend class List<ObjectPtr<T>>;
        ObjectPtr(const ObjectPtrBase& ptr) : base(ptr) {}
    public:
        template<typename U> requires std::is_base_of_v<T, U>
        ObjectPtr(const ObjectPtr<U>& derived) noexcept : base(derived)
        {
        }
        ObjectPtr(const ObjectPtr&) = default;
        ObjectPtr(ObjectPtr&&) = default;
        ObjectPtr() = default;

        ObjectPtr(std::nullptr_t) {}


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

        bool operator==(const ObjectPtrBase& r) const { return ManagedPtr == r.ManagedPtr; }

        [[always_inline]] bool operator==(std::nullptr_t) const noexcept { return !IsValid(); }
        template<typename U>
        bool operator==(const ObjectPtr<U>& r) const noexcept { return ManagedPtr == r.ManagedPtr; }

        template <typename U> requires std::is_base_of_v<T, U>
        ObjectPtr<T>& operator=(const ObjectPtr<U>& o)
        {
            ManagedPtr = o.ManagedPtr;
            return *this;
        }

        ObjectPtr<T>& operator=(const ObjectPtr<T>&) = default;


        static ObjectPtr<T> UnsafeCreate(ObjectHandle h)
        {
            ObjectPtr<T> obj;
            obj.ManagedPtr = RuntimeObjectManager::GetPointer(h);
            return obj;
        }
    };


    template <typename T>
    ObjectPtr<T> cast(const ObjectPtrBase& o)
    {
        if (auto ptr = o.GetObjectPointer())
        {
            if (auto casted = ptr_cast<T>(ptr))
            {
                return ObjectPtr<T>::UnsafeCreate(ptr->GetObjectHandle());
            }
        }
        return {};
    }


    inline void DestroyObject(const ObjectPtrBase& object, bool isForce = false) noexcept
    {
        RuntimeObjectManager::DestroyObject(object.GetHandle(), isForce);
    }

}

#define DECL_OBJECTPTR_SELF        \
    ObjectPtr<ThisClass> self_ptr() const \
    { \
        return ObjectPtr<ThisClass>::UnsafeCreate(GetObjectHandle()); \
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