#pragma once
#include "ObjectBase.h"
#include <CoreLib.Serialization/ObjectSerializer.h>

namespace pulsar
{
    class Scene;
    class NodeCollection;

    class SceneObject : public ObjectBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SceneObject, ObjectBase);
    public:

        void SceneObjectConstruct(guid_t guid = {});

        guid_t GetSceneObjectGuid() const { return m_sceneObjectGuid; }

        virtual void GetDependenciesAsset(array_list<guid_t>& deps) const {}


        virtual NodeCollection* GetOwnerNodeCollection() const = 0;
    protected:
        guid_t m_sceneObjectGuid;
    public:
        guid_t m_sourceGuidInTemplate; // 在模板（prefab）里对应的源对象 GUID，为空表示不是实例化对象
        const guid_t& GetSourceGuidInTemplate() const { return m_sourceGuidInTemplate; }
    };


    class ISceneObjectFinder
    {
    public:
        virtual ~ISceneObjectFinder() = default;
        virtual ObjectPtr<SceneObject> FindSceneObject(guid_t sceneObjId) const = 0;
        virtual void AddSceneObjectToFinder(const guid_t& guid, const ObjectPtr<SceneObject>& obj) = 0;

        void AddSceneObjectToFinder(const ObjectPtr<SceneObject>& obj)
        {
            AddSceneObjectToFinder(obj->GetSceneObjectGuid(), obj);
        }
    };


    struct SceneObjectSerializer
    {
        SceneObjectSerializer(ser::VarientRef obj, bool isWrite, bool editorData,
                              ISceneObjectFinder* sceneObjectFinder = nullptr)
            : Object(std::move(obj)),
              IsWrite(isWrite),
              HasEditorData(editorData),
              SceneObjectFinder(sceneObjectFinder)
        {
        }

        SceneObjectSerializer(const SceneObjectSerializer&) = delete;
        SceneObjectSerializer(SceneObjectSerializer&&) = delete;
    public:
        ISceneObjectFinder* SceneObjectFinder;
        ser::VarientRef Object;
        const bool IsWrite;
        const bool HasEditorData;
    };

    struct SceneObjectPtrBase
    {
        SPtr<ManagedPointer> ManagedPtr;

        SceneObjectPtrBase() = default;
        SceneObjectPtrBase(const SceneObjectPtrBase&) = default;
        SceneObjectPtrBase(SceneObjectPtrBase&&) = default;
        SceneObjectPtrBase(const ObjectHandle& handle)
        {
            ManagedPtr = RuntimeObjectManager::GetPointer(handle);
        }
        explicit SceneObjectPtrBase(std::nullptr_t) {}

        SceneObjectPtrBase& operator=(const SceneObjectPtrBase&) = default;
        SceneObjectPtrBase& operator=(SceneObjectPtrBase&&) = default;

        SceneObject* operator->() const
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
        [[always_inline]] SceneObject* GetObjectPointer() const
        {
            if (ManagedPtr)
            {
                return static_cast<SceneObject*>(ManagedPtr->Get());
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

        static SceneObjectPtrBase UnsafeCreate(ObjectHandle handle)
        {
            return SceneObjectPtrBase{ handle };
        }
    };


    template<typename T>
    struct SceneObjectPtr : public SceneObjectPtrBase
    {
        using base = SceneObjectPtrBase;
        using element_type = T;
    private:
        friend class List<SceneObjectPtr<T>>;
        SceneObjectPtr(const SceneObjectPtrBase& ptr) : base(ptr) {}
    public:
        template<typename U> requires std::is_base_of_v<T, U>
        SceneObjectPtr(const SceneObjectPtr<U>& derived) noexcept : base(derived) {}

        // 从 ObjectPtr<T> 隐式构造
        SceneObjectPtr(const ObjectPtr<T>& obj) noexcept
        {
            if (obj) ManagedPtr = RuntimeObjectManager::GetPointer(obj.GetHandle());
        }
        template<typename U> requires std::is_base_of_v<T, U>
        SceneObjectPtr(const ObjectPtr<U>& obj) noexcept
        {
            if (obj) ManagedPtr = RuntimeObjectManager::GetPointer(obj.GetHandle());
        }

        // 转换回 ObjectPtr<T>
        operator ObjectPtr<T>() const noexcept
        {
            if (auto* ptr = GetPtr())
                return ObjectPtr<T>::UnsafeCreate(ptr->GetObjectHandle());
            return {};
        }

        SceneObjectPtr(const SceneObjectPtr&) = default;
        SceneObjectPtr(SceneObjectPtr&&) = default;
        SceneObjectPtr() = default;
        SceneObjectPtr(std::nullptr_t) {}


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

        bool operator==(const SceneObjectPtrBase& r) const { return ManagedPtr == r.ManagedPtr; }

        [[always_inline]] bool operator==(std::nullptr_t) const noexcept { return !IsValid(); }
        template<typename U>
        bool operator==(const SceneObjectPtr<U>& r) const noexcept { return ManagedPtr == r.ManagedPtr; }

        template <typename U> requires std::is_base_of_v<T, U>
        SceneObjectPtr<T>& operator=(const SceneObjectPtr<U>& o)
        {
            ManagedPtr = o.ManagedPtr;
            return *this;
        }

        SceneObjectPtr<T>& operator=(const SceneObjectPtr<T>&) = default;


        static SceneObjectPtr<T> UnsafeCreate(ObjectHandle h)
        {
            SceneObjectPtr<T> obj;
            obj.ManagedPtr = RuntimeObjectManager::GetPointer(h);
            return obj;
        }
    };


    template <typename T>
    SceneObjectPtr<T> cast(const SceneObjectPtrBase& o)
    {
        if (auto ptr = o.GetObjectPointer())
        {
            if (auto casted = ptr_cast<T>(ptr))
            {
                return SceneObjectPtr<T>::UnsafeCreate(ptr->GetObjectHandle());
            }
        }
        return {};
    }


    inline void DestroyObject(const SceneObjectPtrBase& object, bool isForce = false) noexcept
    {
        RuntimeObjectManager::DestroyObject(object.GetHandle(), isForce);
    }


    class BoxingSceneObjectPtrBase : public PointerBoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxingSceneObjectPtrBase, PointerBoxingObject);
        CORELIB_IMPL_INTERFACES(IStringify)
    public:
        using unboxing_type = SceneObjectPtrBase;
        SceneObjectPtrBase get_unboxing_value() { return ptr; }
        BoxingSceneObjectPtrBase() : CORELIB_INIT_INTERFACE(IStringify) {}
        explicit BoxingSceneObjectPtrBase(const SceneObjectPtrBase& invalue) : ptr(invalue),
                                                                     CORELIB_INIT_INTERFACE(IStringify) {}

        SceneObject* GetPointer() const override { return ptr.GetObjectPointer(); }

        static void Parse(const string& value, guid_t& collection, guid_t& sceneObjId);

        void IStringify_Parse(const string& value) override;
        string IStringify_Stringify() override;

        SceneObjectPtrBase ptr;
    };

}

CORELIB_DECL_BOXING(pulsar::SceneObjectPtrBase, pulsar::BoxingSceneObjectPtrBase);

namespace jxcorlib
{
    template<typename T>
    struct type_redirect<pulsar::SceneObjectPtr<T>>
    {
        using type = pulsar::SceneObjectPtrBase;
    };

    template<typename T>
    struct type_wrapper<pulsar::SceneObjectPtr<T>>
    {
        using type = T;
    };

}

namespace std
{
    template<>
    struct hash<pulsar::SceneObjectPtrBase>
    {
        size_t operator()(const pulsar::SceneObjectPtrBase& ptr) const noexcept
        {
            return std::hash<pulsar::ObjectHandle>()(ptr.GetHandle());
        }
    };

    template<typename T>
    struct hash<pulsar::SceneObjectPtr<T>>
    {
        size_t operator()(const pulsar::SceneObjectPtr<T>& ptr) const noexcept
        {
            return std::hash<pulsar::ObjectHandle>()(ptr.GetHandle());
        }
    };

}