#include <CoreLib/Guid.h>
#include <Pulsar/ObjectBase.h>
#include <map>

namespace pulsar
{

    namespace
    {
        struct RuntimeObjectPointerValue
        {
            sptr<ObjectBase> OriginalObject;
            [[not_null]] sptr<ManagedPointer> PointerRef;

            [[always_inline]] bool IsValid() const
            {
                return OriginalObject != nullptr;
            }
        };
    }

    using _object_table_t = hash_map<ObjectHandle, RuntimeObjectPointerValue>;
    static auto& _object_table()
    {
        static _object_table_t table;
        return table;
    }

    Action<ObjectBase*> RuntimeObjectWrapper::OnPostEditChanged{};

    static inline ObjectHandle _NewId()
    {
        return ObjectHandle::create_new();
    }


    ObjectBase* RuntimeObjectWrapper::GetObject(const ObjectHandle& id) noexcept
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

    sptr<ObjectBase> RuntimeObjectWrapper::GetSharedObject(const ObjectHandle& id) noexcept
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
    sptr<ManagedPointer> RuntimeObjectWrapper::GetPointer(const ObjectHandle& id) noexcept
    {
        auto it = _object_table().find(id);
        if (it != _object_table().end())
        {
            return it->second.PointerRef;
        }
        return nullptr;
    }

    sptr<ManagedPointer> RuntimeObjectWrapper::AddWaitPointer(const ObjectHandle& id)
    {
        RuntimeObjectPointerValue value{};
        value.PointerRef = mksptr(new ManagedPointer{});
        _object_table().insert({id, value});
        return value.PointerRef;
    }

    bool RuntimeObjectWrapper::IsValid(const ObjectHandle& id) noexcept
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

    void RuntimeObjectWrapper::NewInstance(sptr<ObjectBase>&& managedObj, const ObjectHandle& handle) noexcept
    {
        const auto id = handle.is_empty() ? _NewId() : handle;
        managedObj->m_objectHandle = id;

        auto it = _object_table().find(handle);
        if (it != _object_table().end())
        {
            assert(!it->second.IsValid());
            it->second.OriginalObject = std::move(managedObj);
            it->second.PointerRef->Pointer = it->second.OriginalObject.get();
        }
        else
        {
            RuntimeObjectPointerValue value;
            value.OriginalObject = std::move(managedObj);
            value.PointerRef = mksptr(new ManagedPointer{});
            value.PointerRef->Pointer = value.OriginalObject.get();

            _object_table().emplace(id, value);
        }

    }

    bool RuntimeObjectWrapper::DestroyObject(const ObjectHandle& id, bool isForce) noexcept
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
                    // set null pointer
                    it->second.PointerRef->SetEmptyPtr();
                    _object_table().erase(id);
                    return true;
                }
            }

        }
        return false;
    }
    void RuntimeObjectWrapper::Terminate()
    {
        array_list<ObjectHandle> destroyList;
        for (const auto& [id, ptr] : _object_table())
        {
            if (ptr.IsValid())
            {
                if (!ptr.OriginalObject->HasObjectFlags(OF_LifecycleManaged))
                {
                    continue;
                }
                destroyList.push_back(id);
            }
        }
        for(auto& id : destroyList)
        {
            DestroyObject(id);
        }

        // release memory
        _object_table_t{}.swap(_object_table());
    }
    void RuntimeObjectWrapper::ForEachObject(const std::function<void(ObjectHandle, ObjectBase*)>& func)
    {
        for (auto& item : _object_table())
        {
            func(item.first, item.second.OriginalObject.get());
        }
    }

    static auto& _depends()
    {
        static hash_map<ObjectHandle, array_list<ObjectHandle>> map;
        return map;
    }

    void RuntimeObjectWrapper::AddDependList(ObjectHandle src, ObjectHandle dest)
    {
        _depends()[dest].emplace_back(src);
    }
    void RuntimeObjectWrapper::NotifyDependObjects(ObjectHandle dest, int id)
    {
        for (auto srcId : _depends()[dest])
        {
            if (ObjectPtr<ObjectBase> src{srcId})
            {
                src->OnDependencyMessage(dest, id);
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
        RuntimeObjectWrapper::NewInstance(self(), handle);
        this->OnConstruct();
    }

    void ObjectBase::PostEditChange(FieldInfo* info)
    {
        RuntimeObjectWrapper::OnPostEditChanged.Invoke(this);
    }
    void ObjectBase::SetIndexName(index_string name) noexcept
    {
        m_name = name;
    }
    void ObjectBase::SetName(string_view name) noexcept
    {
        SetIndexName(name);
    }

    void ObjectBase::OnDependencyMessage(ObjectHandle inDependency, int msg)
    {

    }
    void ObjectBase::AddOutDependency(ObjectHandle obj)
    {
        if (!m_outDependency)
        {
            m_outDependency = std::make_unique<array_list<ObjectHandle>>();
        }
        if (!std::ranges::contains(*m_outDependency, obj))
        {
            m_outDependency->push_back(obj);
        }
    }
    bool ObjectBase::HasOutDependency(ObjectHandle obj) const
    {
        if (m_outDependency)
        {
            return std::ranges::contains(*m_outDependency, obj);
        }
        return false;
    }
    void ObjectBase::RemoveOutDependency(ObjectHandle obj)
    {
        if (m_outDependency)
        {
            auto it = std::remove(m_outDependency->begin(), m_outDependency->end(), obj);
            if (it != m_outDependency->end())
            {
                m_outDependency->pop_back();
            }
        }
    }
    void ObjectBase::SendMsgToOutDependency(int msg) const
    {
        if (m_outDependency)
        {
            for (auto& element : *m_outDependency)
            {
                if (auto obj = ObjectPtr<ObjectBase>{element})
                {
                    obj->OnDependencyMessage(m_objectHandle, msg);
                }
            }
        }
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
        SendMsgToOutDependency(DependMsg_OnDestroy);
    }

    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, ObjectPtrBase& obj)
    {
        if (!isWrite)
        {
            obj.Pointer->SetEmptyPtr();
        }
        sser::ReadWriteStream(stream, isWrite, obj.GetHandle().x);
        sser::ReadWriteStream(stream, isWrite, obj.GetHandle().y);
        sser::ReadWriteStream(stream, isWrite, obj.GetHandle().z);
        sser::ReadWriteStream(stream, isWrite, obj.GetHandle().w);
        return stream;
    }
    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, RCPtrBase& obj)
    {
        if (!isWrite)
        {
            obj.Pointer->SetEmptyPtr();
        }
        sser::ReadWriteStream(stream, isWrite, obj.GetHandle().x);
        sser::ReadWriteStream(stream, isWrite, obj.GetHandle().y);
        sser::ReadWriteStream(stream, isWrite, obj.GetHandle().z);
        sser::ReadWriteStream(stream, isWrite, obj.GetHandle().w);
        return stream;
    }
} // namespace pulsar