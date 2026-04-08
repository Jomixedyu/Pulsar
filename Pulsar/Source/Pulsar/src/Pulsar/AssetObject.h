#pragma once

#include <Pulsar/ObjectBase.h>
#include <CoreLib/Guid.h>
#include <CoreLib/sser.hpp>
#include <CoreLib.Serialization/DataSerializer.h>
#include <CoreLib.Serialization/ObjectSerializer.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <memory>
#include <utility>



namespace pulsar
{
    class CreateAssetAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CreateAssetAttribute, Attribute);

    public:
        CreateAssetAttribute() = default;
        explicit CreateAssetAttribute(const char* instantiatePath) : m_instantiatePath(instantiatePath)
        {
        }
        auto GetInstantiatePath() const noexcept
        {
            return m_instantiatePath;
        }

    private:
        const char* m_instantiatePath = nullptr;
    };

    struct AssetSerializer
    {
        // 磁盘文件模式：懒打开 .pba 文件
        AssetSerializer(ser::VarientRef obj, std::filesystem::path binaryPath, bool isWrite, bool editorData)
            : Object(std::move(obj)),
              m_binaryPath(std::move(binaryPath)),
              IsWrite(isWrite),
              HasEditorData(editorData)
        {
        }

        // 内存流模式：直接使用外部提供的流（用于资产克隆等场景）
        AssetSerializer(ser::VarientRef obj, std::iostream& externalStream, bool isWrite, bool editorData)
            : Object(std::move(obj)),
              m_externalStream(&externalStream),
              IsWrite(isWrite),
              HasEditorData(editorData)
        {
        }

        AssetSerializer(const AssetSerializer&) = delete;
        AssetSerializer(AssetSerializer&&) = delete;

        // 懒访问器：首次调用时按需打开/创建 .pba 文件
        std::iostream& GetStream()
        {
            // 内存流模式
            if (m_externalStream)
                return *m_externalStream;

            // 磁盘文件懒初始化
            if (!m_stream)
            {
                if (IsWrite)
                {
                    m_stream = std::make_unique<std::fstream>(
                        m_binaryPath, std::ios::out | std::ios::trunc | std::ios::binary);
                }
                else
                {
                    m_stream = std::make_unique<std::fstream>(
                        m_binaryPath, std::ios::in | std::ios::binary);
                }
            }
            return *m_stream;
        }

        // 查询流是否可用
        bool HasStream() const
        {
            if (m_externalStream)
                return true;
            if (IsWrite)
                return m_stream != nullptr;
            else
                return std::filesystem::exists(m_binaryPath);
        }

    public:
        ser::VarientRef Object;
        OSPlatform Platform;
        bool CookedOnly;
        const bool IsWrite;
        const bool HasEditorData;

    private:
        std::filesystem::path m_binaryPath;
        std::unique_ptr<std::fstream> m_stream;
        std::iostream* m_externalStream = nullptr;
    };

    class ImportedFileInfo : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ImportedFileInfo, Object);

    public:
        CORELIB_REFL_DECL_FIELD(m_filename);
        String_sp m_filename;
        CORELIB_REFL_DECL_FIELD(m_latestModification);
        String_sp m_latestModification;
        CORELIB_REFL_DECL_FIELD(m_hash);
        String_sp m_hash;

        ImportedFileInfo()
        {
            init_sptr_member(m_filename);
            init_sptr_member(m_latestModification);
            init_sptr_member(m_hash);
        }
    };
    CORELIB_DECL_SHORTSPTR(ImportedFileInfo);

    class AssetObject : public ObjectBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::AssetObject, ObjectBase);
        friend class RuntimeAssetManager;
        using base::Construct;
    public:
        void AssetConstruct(guid_t guid = {});

        virtual void Serialize(AssetSerializer* s);
        virtual bool CanInstantiateAsset() const
        {
            return true;
        }

        virtual void OnCollectAssetDependencies(array_list<guid_t>& deps)
        {
        }


    protected:
        void Destroy() override;

    public:
        AssetObject();
        AssetObject(const AssetObject&) = delete;
        AssetObject(AssetObject&&) = delete;
        AssetObject& operator=(const AssetObject&) = delete;

        guid_t GetAssetGuid() const
        {
            return m_assetGuid;
        }

    public:
        virtual void OnInstantiateAsset(AssetObject* obj);

    protected:
#ifdef WITH_EDITOR
        CORELIB_REFL_DECL_FIELD(m_importFiles);
        List_sp<ImportedFileInfo_sp> m_importFiles;
#endif

        CORELIB_REFL_DECL_FIELD(m_tags);
        List_sp<String_sp> m_tags;

        CORELIB_REFL_DECL_FIELD(m_assetGuid);
        guid_t m_assetGuid;
    };
    DECL_PTR(AssetObject);

    template <typename T>
    concept baseof_assetobject = std::is_base_of_v<AssetObject, T>;

    class RCPtrBase
    {
    protected:
        SPtr<ManagedPointer> ManagedPtr;

        template <typename T>
        friend struct RCPtr;

    protected:
        [[always_inline]] void Incref() const noexcept
        {
            if (ManagedPtr)
            {
                ManagedPtr->Incref();
            }
        }
        [[always_inline]] void Decref()
        {
            if (ManagedPtr && ManagedPtr->Decref() == 0)
            {
                if (!ManagedPtr->Get()) //destroyed
                {
                    return;
                }
                RuntimeObjectManager::DestroyObject(ManagedPtr->Get()->GetObjectHandle());
            }
        }

    protected:
        //unsafe! ignore type
        RCPtrBase(const ObjectHandle& handle)
        {
            ManagedPtr = RuntimeObjectManager::GetPointer(handle);
            Incref();
        }
    public:
        RCPtrBase() = default;
        RCPtrBase(const RCPtrBase& ptr) noexcept : ManagedPtr(ptr.ManagedPtr)
        {
            Incref();
        }
        RCPtrBase(RCPtrBase&& ptr) noexcept : ManagedPtr(std::move(ptr.ManagedPtr))
        {
            ptr.ManagedPtr = nullptr;
        }

        template <baseof_assetobject T>
        RCPtrBase(const ObjectPtr<T>& obj) : ManagedPtr()
        {
            if (obj)
            {
                ManagedPtr = RuntimeObjectManager::GetPointer(obj.GetHandle());
            }
            Incref();
        }
        RCPtrBase(std::nullptr_t) {}
        ~RCPtrBase() noexcept
        {
            if (ManagedPtr)
            {
                Decref();
            }
        }

        [[always_inline]] ObjectBase* GetPointer() const noexcept
        {
            if (ManagedPtr)
            {
                return ManagedPtr->Get();
            }
            return nullptr;
        }
        [[always_inline]] AssetObject* GetAssetPointer() const noexcept
        {
            if (ManagedPtr)
            {
                return static_cast<AssetObject*>(ManagedPtr->Get());
            }
            return nullptr;
        }
        ObjectHandle GetHandle() const noexcept
        {
            if (auto ptr = GetPointer())
            {
                return ptr->GetObjectHandle();
            }
            return {};
        }
        guid_t GetGuid() const noexcept
        {
            if (auto ptr = GetAssetPointer())
            {
                return ptr->GetAssetGuid();
            }
            return {};
        }
        RCPtrBase& operator=(const RCPtrBase& ptr) noexcept
        {
            if (this == &ptr)
                return *this;
            Decref();
            ManagedPtr = ptr.ManagedPtr;
            Incref();
            return *this;
        }
        RCPtrBase& operator=(RCPtrBase&& ptr) noexcept
        {
            if (this == &ptr)
                return *this;
            Decref();
            ManagedPtr = std::move(ptr.ManagedPtr);
            return *this;
        }
        AssetObject* operator->() const
        {
            auto ptr = GetAssetPointer();
            if (!ptr) throw NullPointerException();
            return ptr;
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
            return ManagedPtr == ptr.ManagedPtr;
        }

        bool operator==(std::nullptr_t) const noexcept
        {
            return !IsValid();
        }

        ObjectPtrBase ToObjectPtr() const
        {
            return ObjectPtrBase{ GetHandle() };
        }

        void Reset()
        {
            Decref();
            ManagedPtr.reset();
        }

        bool IsEmpty() const
        {
            return ManagedPtr == nullptr;
        }

        static RCPtrBase UnsafeCreate(ObjectHandle handle)
        {
            return RCPtrBase{ handle };
        }

        static RCPtrBase SafeCreate(ObjectHandle handle)
        {
            ObjectPtrBase objPtr = handle;
            if (auto o = cast<AssetObject>(objPtr))
            {
                return RCPtrBase(o);
            }
            return {};
        }
    };

    template <typename T>
    struct RCPtr : public RCPtrBase
    {
        friend class List<RCPtr<T>>;
    protected:
        using base = RCPtrBase;
    private:
        //unsafe! ignore type
        RCPtr(const RCPtrBase& ptr) : RCPtrBase(ptr)
        {
        }
        RCPtr(ObjectHandle handle) : RCPtrBase(handle)
        {
        }
    public:
        T* operator->() const
        {
            auto ptr = GetPtr();
            if (!ptr)
            {
                throw NullPointerException();
            }
            return ptr;
        }
        RCPtr() = default;
        RCPtr(std::nullptr_t)   { }
        RCPtr(const RCPtr<T>& ptr) : RCPtrBase(ptr)   {   }
        RCPtr(RCPtr<T>&& ptr) : RCPtrBase(std::move(ptr))   {  }

        template <typename U>
            requires std::is_base_of_v<T, U>
        RCPtr(const RCPtr<U>& derived) noexcept : RCPtrBase(derived)
        {
        }

        RCPtr<T>& operator=(const RCPtr<T>& ptr) noexcept
        {
            if (ManagedPtr == ptr.ManagedPtr)
                return *this;
            Decref();
            ManagedPtr = ptr.ManagedPtr;
            Incref();
            return *this;
        }


        template <typename U> requires std::is_base_of_v<T, U>
        RCPtr<T>& operator=(const RCPtr<U>& ptr) noexcept
        {
            if (ManagedPtr == ptr.ManagedPtr)
                return *this;
            Decref();
            ManagedPtr = ptr.ManagedPtr;
            Incref();
            return *this;
        }

        RCPtr<T>& operator=(RCPtr<T>&& ptr) noexcept
        {
            if (ManagedPtr == ptr.ManagedPtr)
                return *this;
            ManagedPtr = ptr.ManagedPtr;
            ptr.ManagedPtr = nullptr;
            return *this;
        }

        T* GetPtr() const noexcept
        {
            return static_cast<T*>(GetPointer());
        }

        operator ObjectPtr<T>() const
        {
            return cast<T>(ObjectPtrBase{GetHandle()});
        }

        static RCPtr<T> UnsafeCreate(ObjectHandle o)
        {
            return RCPtr<T>{ o };
        }
    };

    template <baseof_assetobject T>
    RCPtr<T> cast(const RCPtrBase& o)
    {
        if (auto ptr = o.GetPointer())
        {
            if (auto casted = ptr_cast<T>(ptr))
            {
                return RCPtr<T>::UnsafeCreate(casted->GetObjectHandle());
            }
        }
        return {};
    }

    RCPtrBase InternalNewAssetObject(Type* type, guid_t assetGuid);

    inline RCPtrBase NewAssetObject(Type* type)
    {
        return InternalNewAssetObject(type, {});
    }

    template <baseof_assetobject T>
    RCPtr<T> NewAssetObject(Type* type = cltypeof<T>())
    {
        return cast<T>(NewAssetObject(type));
    }

    class BoxingRCPtrBase : public PointerBoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxingRCPtrBase, PointerBoxingObject);
        CORELIB_IMPL_INTERFACES(IStringify)
    public:
        using unboxing_type = RCPtrBase;
        RCPtrBase get_unboxing_value()
        {
            return ptr;
        }
        BoxingRCPtrBase() : CORELIB_INIT_INTERFACE(IStringify), ptr()
        {
        }
        explicit BoxingRCPtrBase(const RCPtrBase& invalue) : CORELIB_INIT_INTERFACE(IStringify), ptr(invalue)
        {
        }

        Object* GetPointer() const override
        {
            return ptr.GetPointer();
        }

        void IStringify_Parse(const string& value) override;

        string IStringify_Stringify() override;

        [[always_inline]] ObjectHandle GetHandle() const noexcept
        {
            auto p = ptr.GetPointer();
            ObjectHandle h{};
            if (p)
            {
                h = p->GetObjectHandle();
            }
            return h;
        }
        [[always_inline]] void SetHandle(const ObjectHandle& handle) noexcept
        {
            ptr = RCPtrBase::UnsafeCreate(handle);
        }

        RCPtrBase ptr;
    };
}
CORELIB_DECL_BOXING(pulsar::RCPtrBase, pulsar::BoxingRCPtrBase);

namespace jxcorlib
{
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
namespace std
{
    template<>
    struct hash<pulsar::RCPtrBase>
    {
        size_t operator()(const pulsar::RCPtrBase& ptr) const noexcept
        {
            return std::hash<pulsar::ObjectBase*>()(ptr.GetPointer());
        }
    };

    template<typename T>
    struct hash<pulsar::RCPtr<T>>
    {
        size_t operator()(const pulsar::RCPtr<T>& ptr) const noexcept
        {
            return std::hash<pulsar::ObjectBase*>()(ptr.GetPointer());
        }
    };
}

namespace pulsar
{
    RCPtrBase InstantiateAsset(const RCPtrBase& asset);

    template <baseof_assetobject T>
    RCPtr<T> InstantiateAsset(const RCPtr<T>& asset)
    {
        const RCPtrBase& a = asset;
        return cast<T>(InstantiateAsset(a));
    }

    class RuntimeAssetManager
    {
    public:

        static void Register(AssetObject* obj);
        static void Unregister(AssetObject* obj);

        static RCPtrBase GetLoadedAssetByGuid(guid_t id);
        static ObjectHandle GetLoadedAssetHandleByGuid(guid_t id);
        static bool IsLoaded(guid_t guid);

        template <typename T>
        static RCPtr<T> GetLoadedAssetByGuid(guid_t id)
        {
            return cast<T>(GetLoadedAssetByGuid(id));
        }
    };

    inline void DestroyObject(const RCPtrBase& object, bool isForce = false)
    {
        DestroyObject( object.ToObjectPtr(), isForce);
    }

    template <typename T>
    struct WeakAssetPtr
    {
        guid_t AssetGuid;
        WeakAssetPtr() = default;
        WeakAssetPtr(guid_t guid) : AssetGuid(guid) {}
        WeakAssetPtr(const std::string& v)
        {
            AssetGuid = guid_t::parse(v);
        }

        bool IsLoaded() const
        {
            return RuntimeAssetManager::GetLoadedAssetByGuid(AssetGuid).IsValid();
        }
        bool IsEmpty() const
        {
            return AssetGuid.is_empty();
        }
        RCPtr<T> Get() const
        {
            return RuntimeAssetManager::GetLoadedAssetByGuid<T>(AssetGuid);
        }

        std::string ToString() const
        {
            return AssetGuid.to_string();
        }
    };
}
