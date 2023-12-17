#include <Pulsar/ObjectBase.h>
#include <CoreLib/Guid.h>
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

    ObjectBase* RuntimeObjectWrapper::GetObject(ObjectHandle id)
    {
        if (id.is_empty()) return nullptr;
        auto it = _object_table()->find(id);
        if (it != _object_table()->end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    sptr<ObjectBase> RuntimeObjectWrapper::GetSharedObject(ObjectHandle id)
    {
        if (id.is_empty()) return nullptr;
        auto it = _object_table()->find(id);
        if (it != _object_table()->end())
        {
            return it->second;
        }
        return nullptr;
    }

    bool RuntimeObjectWrapper::IsValid(ObjectHandle id)
    {
        if (id.is_empty()) return false;
        return _object_table()->find(id) != _object_table()->end();
    }
    void RuntimeObjectWrapper::NewInstance(sptr<ObjectBase>&& managedObj, ObjectHandle handle)
    {
        const auto id = handle.is_empty() ? _NewId() : handle;
        managedObj->m_objectHandle = id;
        _object_table()->emplace(id, std::move(managedObj));
    }

    void RuntimeObjectWrapper::DestroyObject(ObjectHandle id, bool isForce)
    {
        if (id.is_empty()) return;

        if(auto obj = GetObject(id))
        {
            const bool persistent = obj->HasObjectFlags(OF_Persistent);
            if (!persistent || (persistent && isForce))
            {
                obj->Destroy();
                _object_table()->erase(id);
            }
        }
    }
    void RuntimeObjectWrapper::Terminate()
    {
        auto map = *_object_table();
        for (auto& [id, ptr] : map)
        {
            DestroyObject(id, true);
        }
        // release memory
        std::remove_reference_t<decltype(*_object_table())>{}.swap(*_object_table());
    }
    void RuntimeObjectWrapper::ForEachObject(const std::function<void(ObjectHandle, ObjectBase*)>& func)
    {
        for(auto& item : *_object_table())
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
    void ObjectBase::SetIndexName(index_string name)
    {
        m_name = name;
        #ifdef WITH_EDITOR
        m_objectName = name.to_string();
        #endif
    }
    void ObjectBase::SetName(string_view name)
    {
        SetIndexName(name);
    }

    void ObjectBase::OnConstruct()
    {
    }

    ObjectBase::~ObjectBase()
    {
        if (m_objectHandle)
        {
            //throw EngineException("not destory");
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


    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, ObjectPtrBase& obj)
    {
        sser::ReadWriteStream(stream, isWrite, obj.handle.x);
        sser::ReadWriteStream(stream, isWrite, obj.handle.y);
        sser::ReadWriteStream(stream, isWrite, obj.handle.z);
        sser::ReadWriteStream(stream, isWrite, obj.handle.w);
        return stream;
    }
}