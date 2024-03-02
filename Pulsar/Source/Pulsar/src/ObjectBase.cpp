#include <CoreLib/Guid.h>
#include <Pulsar/ObjectBase.h>
#include <map>

namespace pulsar
{

    static auto _object_table()
    {
        static auto table = new hash_map<ObjectHandle, sptr<ObjectBase>>;
        return table;
    }

    Action<ObjectBase*> RuntimeObjectWrapper::OnPostEditChanged{};

    static inline ObjectHandle _NewId()
    {
        return ObjectHandle::create_new();
    }

    ObjectBase* RuntimeObjectWrapper::GetObject(ObjectHandle id) noexcept
    {
        if (id.is_empty())
            return nullptr;
        auto it = _object_table()->find(id);
        if (it != _object_table()->end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    sptr<ObjectBase> RuntimeObjectWrapper::GetSharedObject(ObjectHandle id) noexcept
    {
        if (id.is_empty())
            return nullptr;
        auto it = _object_table()->find(id);
        if (it != _object_table()->end())
        {
            return it->second;
        }
        return nullptr;
    }

    bool RuntimeObjectWrapper::IsValid(ObjectHandle id) noexcept
    {
        if (id.is_empty())
            return false;
        return _object_table()->find(id) != _object_table()->end();
    }
    void RuntimeObjectWrapper::NewInstance(sptr<ObjectBase>&& managedObj, ObjectHandle handle) noexcept
    {
        const auto id = handle.is_empty() ? _NewId() : handle;
        managedObj->m_objectHandle = id;
        _object_table()->emplace(id, std::move(managedObj));
    }

    bool RuntimeObjectWrapper::DestroyObject(ObjectHandle id, bool isForce) noexcept
    {
        if (id.is_empty())
            return false;

        if (auto obj = GetObject(id))
        {
            const bool dontDestory = obj->HasObjectFlags(OF_LifecycleManaged);
            if (!dontDestory || (dontDestory && isForce))
            {
                obj->Destroy();
                _object_table()->erase(id);
                return true;
            }
        }
        return false;
    }
    void RuntimeObjectWrapper::Terminate()
    {
        array_list<ObjectHandle> destroyList;
        for (const auto& [id, ptr] : *_object_table())
        {
            if (!ptr->HasObjectFlags(OF_LifecycleManaged))
            {
                continue;
            }
            destroyList.push_back(id);
        }
        for(auto& id : destroyList)
        {
            DestroyObject(id);
        }

        // release memory
        std::remove_reference_t<decltype(*_object_table())>{}.swap(*_object_table());
    }
    void RuntimeObjectWrapper::ForEachObject(const std::function<void(ObjectHandle, ObjectBase*)>& func)
    {
        for (auto& item : *_object_table())
        {
            func(item.first, item.second.get());
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
        sser::ReadWriteStream(stream, isWrite, obj.handle.x);
        sser::ReadWriteStream(stream, isWrite, obj.handle.y);
        sser::ReadWriteStream(stream, isWrite, obj.handle.z);
        sser::ReadWriteStream(stream, isWrite, obj.handle.w);
        return stream;
    }
} // namespace pulsar