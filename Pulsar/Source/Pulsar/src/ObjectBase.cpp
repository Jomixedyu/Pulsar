#include <CoreLib/Guid.h>
#include <Pulsar/ObjectBase.h>
#include <map>
#include <ranges>

namespace pulsar
{

    namespace
    {
        struct RuntimeObjectPointerValue
        {
            SPtr<ObjectBase> OriginalObject;
            [[not_null]] SPtr<ManagedPointer> ManagedPtr;

            [[always_inline]] bool IsValid() const
            {
                return OriginalObject != nullptr;
            }

            void Destroy()
            {
                OriginalObject.reset();
                ManagedPtr->SetEmptyPtr();
            }
        };
    }

    using _object_table_t = hash_map<ObjectHandle, RuntimeObjectPointerValue>;
    static auto& _object_table()
    {
        static _object_table_t table;
        return table;
    }

    Action<ObjectBase*> RuntimeObjectManager::OnPostEditChanged{};

    static inline ObjectHandle _NewId()
    {
        return ObjectHandle::create_new();
    }


    ObjectBase* RuntimeObjectManager::GetObject(const ObjectHandle& id) noexcept
    {
        if (id.is_empty())
            return nullptr;
        auto it = _object_table().find(id);
        if (it != _object_table().end())
        {
            return it->second.OriginalObject.get();
        }
        return nullptr;
    }

    SPtr<ObjectBase> RuntimeObjectManager::GetSharedObject(const ObjectHandle& id) noexcept
    {
        if (id.is_empty())
            return nullptr;
        auto it = _object_table().find(id);
        if (it != _object_table().end())
        {
            return it->second.OriginalObject;
        }
        return nullptr;
    }
    SPtr<ManagedPointer> RuntimeObjectManager::GetPointer(const ObjectHandle& id) noexcept
    {
        auto it = _object_table().find(id);
        if (it != _object_table().end())
        {
            return it->second.ManagedPtr;
        }
        return nullptr;
    }

    SPtr<ManagedPointer> RuntimeObjectManager::AddWaitPointer(const ObjectHandle& id)
    {
        RuntimeObjectPointerValue value{};
        value.ManagedPtr = mksptr(new ManagedPointer{});
        _object_table().insert({id, value});
        return value.ManagedPtr;
    }

    bool RuntimeObjectManager::IsValid(const ObjectHandle& id) noexcept
    {
        if (id.is_empty())
            return false;
        auto it = _object_table().find(id);
        if (it == _object_table().end())
        {
            return false;
        }
        return it->second.IsValid();
    }

    void RuntimeObjectManager::NewInstance(SPtr<ObjectBase>&& managedObj, const ObjectHandle& handle) noexcept
    {
        const auto id = handle.is_empty() ? _NewId() : handle;
        managedObj->m_objectHandle = id;

        auto it = _object_table().find(handle);
        if (it != _object_table().end())
        {
            assert(!it->second.IsValid());
            it->second.OriginalObject = std::move(managedObj);
            it->second.ManagedPtr->Pointer = it->second.OriginalObject.get();
        }
        else
        {
            RuntimeObjectPointerValue value;
            value.OriginalObject = std::move(managedObj);
            value.ManagedPtr = mksptr(new ManagedPointer{});
            value.ManagedPtr->Pointer = value.OriginalObject.get();

            _object_table().emplace(id, value);
        }

    }

    bool RuntimeObjectManager::DestroyObject(const ObjectHandle& id, bool isForce) noexcept
    {
        if (id.is_empty())
            return false;

        auto it = _object_table().find(id);

        if (it != _object_table().end())
        {
            if (const auto obj = it->second.OriginalObject.get())
            {
                const bool dontDestory = obj->HasObjectFlags(OF_LifecycleManaged);
                if (!dontDestory || (dontDestory && isForce))
                {
                    obj->Destroy();
                    it->second.OriginalObject.reset();
                    it->second.ManagedPtr->SetEmptyPtr();
                    if (it->second.ManagedPtr.use_count() == 1)
                    {
                        it->second.ManagedPtr.reset();
                        _object_table().erase(id);
                    }

                    return true;
                }
            }
            else
            {
                if (it->second.ManagedPtr.use_count() == 1)
                {
                    it->second.ManagedPtr.reset();
                    _object_table().erase(id);
                }
                return true;
            }
        }
        return false;
    }
    void RuntimeObjectManager::GetData(size_t* total, size_t* place, size_t* alive)
    {
        size_t t = _object_table().size(), p = 0, a = 0;
        for (auto& [k, v] : _object_table())
        {
            if (v.OriginalObject)
            {
                ++a;
            }
            else
            {
                if (v.ManagedPtr)
                {
                    ++p;
                }
            }
        }
        *total = t;
        *place = p;
        *alive = a;
    }
    void RuntimeObjectManager::Terminate()
    {
        array_list<ObjectHandle> destroyList;
        for (const auto& [id, ptr] : _object_table())
        {
            if (ptr.IsValid())
            {
                if (ptr.OriginalObject->HasObjectFlags(OF_LifecycleManaged))
                {
                    continue;
                }
                destroyList.push_back(id);
            }
        }
        for (auto& id : destroyList)
        {
            DestroyObject(id);
        }

        // release memory
        _object_table_t{}.swap(_object_table());
    }

    void RuntimeObjectManager::TickGCollect()
    {

    }

    void RuntimeObjectManager::ForEachObject(const std::function<void(ObjectHandle, ObjectBase*, int)>& func)
    {
        for (auto& item : _object_table())
        {
            func(item.first, item.second.OriginalObject.get(), item.second.ManagedPtr.use_count());
        }
    }

    static auto& _depends()
    {
        static hash_map<ObjectHandle, array_list<ObjectHandle>> map;
        return map;
    }

    void RuntimeObjectManager::AddDependList(ObjectHandle src, ObjectHandle dest)
    {
        _depends()[dest].emplace_back(src);
    }
    void RuntimeObjectManager::RemoveDependList(ObjectHandle src, ObjectHandle dest)
    {
        std::erase(_depends()[dest], src);
        if (_depends()[dest].empty())
        {
            _depends().erase(dest);
        }
    }
    void RuntimeObjectManager::NotifyDependObjects(ObjectHandle dest, DependencyObjectState id)
    {
        auto it = _depends().find(dest);
        if (it != _depends().end())
        {
            for (const auto& srcId : it->second)
            {
                if (auto src = ObjectPtr<ObjectBase>(srcId).GetPtr())
                {
                    src->OnDependencyMessage(dest, id);
                }
            }
        }

    }

    ObjectBase::ObjectBase()
    {
    }
    void ObjectBase::Construct(ObjectHandle handle)
    {
        assert(this);
        assert(!this->m_objectHandle);
        RuntimeObjectManager::NewInstance(self(), handle);
        this->OnConstruct();
    }

    void ObjectBase::PostEditChange(FieldInfo* info)
    {
        RuntimeObjectManager::OnPostEditChanged.Invoke(this);
    }
    void ObjectBase::SetIndexName(index_string name) noexcept
    {
        m_name = name;
    }
    void ObjectBase::SetName(string_view name) noexcept
    {
        SetIndexName(name);
    }

    void ObjectBase::OnDependencyMessage(ObjectHandle inDependency, DependencyObjectState msg)
    {

    }

    void ObjectBase::OnConstruct()
    {
    }

    ObjectBase::~ObjectBase() noexcept
    {
        if (m_objectHandle)
        {
            // throw EngineException("not destory");
            assert(true);
        }
    }
    void ObjectBase::Destroy()
    {
        this->OnDestroy();
        this->m_objectHandle = ObjectHandle{};
    }

    void ObjectBase::OnDestroy()
    {

    }
    void ObjectBase::SendOuterDependencyMsg(DependencyObjectState msg) const
    {
        RuntimeObjectManager::NotifyDependObjects(GetObjectHandle(), msg);
    }

    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, ObjectPtrBase& obj)
    {
        ObjectHandle handle = obj.Handle;

        sser::ReadWriteStream(stream, isWrite, handle.x);
        sser::ReadWriteStream(stream, isWrite, handle.y);
        sser::ReadWriteStream(stream, isWrite, handle.z);
        sser::ReadWriteStream(stream, isWrite, handle.w);

        if (!isWrite)
        {
            obj = handle;
        }

        return stream;
    }
    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, RCPtrBase& obj)
    {
        ObjectHandle handle = obj.Handle;

        sser::ReadWriteStream(stream, isWrite, handle.x);
        sser::ReadWriteStream(stream, isWrite, handle.y);
        sser::ReadWriteStream(stream, isWrite, handle.z);
        sser::ReadWriteStream(stream, isWrite, handle.w);

        if (!isWrite)
        {
            obj = handle;
        }

        return stream;
    }
} // namespace pulsar