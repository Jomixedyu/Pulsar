#include <Pulsar/ObjectBase.h>
#include <CoreLib/Guid.h>
#include <map>

namespace pulsar
{

    static auto _object_table()
    {
        static auto table = new std::map<ObjectHandle, wptr<ObjectBase>>;
        return table;
    }

    static inline ObjectHandle _NewId()
    {
        return ObjectHandle::create_new();
    }

    bool RuntimeObjectWrapper::GetObject(ObjectHandle id, wptr<ObjectBase>* out)
    {
        if (id.is_empty()) return false;
        auto it = _object_table()->find(id);
        if (it != _object_table()->end())
        {
            *out = it->second;
            return true;
        }
        return false;
    }

    bool RuntimeObjectWrapper::IsValid(ObjectHandle id)
    {
        if (id.is_empty()) return false;
        return _object_table()->find(id) != _object_table()->end();
    }
    void RuntimeObjectWrapper::NewInstance(sptr<ObjectBase> ptr)
    {
        auto id = _NewId();
        ptr->m_objectHandle = id;
        _object_table()->insert(std::pair<ObjectHandle, wptr<ObjectBase>>{ id, wptr<ObjectBase>(ptr) });
    }

    void RuntimeObjectWrapper::DestroyObject(const sptr<ObjectBase>& obj)
    {
        _object_table()->erase(obj->GetObjectHandle());
    }

    void RuntimeObjectWrapper::ForceDestroyObject(ObjectHandle id)
    {
        if (id.is_empty()) return;
        _object_table()->erase(id);
    }

    ObjectBase::ObjectBase()
    {

    }
    void ObjectBase::Construct()
    {
        assert(this);
        assert(!this->m_objectHandle);
        RuntimeObjectWrapper::NewInstance(self());
        this->OnConstruct();
    }
    bool ObjectBase::IsAlive() const
    {
        return RuntimeObjectWrapper::IsValid(this->GetObjectHandle());
    }
    void ObjectBase::OnConstruct()
    {
    }

    ObjectBase::~ObjectBase()
    {
        if (!this->m_objectHandle.is_empty())
        {
            this->Destroy();
        }
    }
    void ObjectBase::Destroy()
    {
        this->OnDestroy();
        RuntimeObjectWrapper::ForceDestroyObject(this->m_objectHandle);
        this->m_objectHandle = ObjectHandle{};
    }

    void ObjectBase::OnDestroy()
    {

    }

    ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, ObjectPtrBase& obj)
    {
        ser::ReadWriteStream(stream, isWrite, obj.handle.x);
        ser::ReadWriteStream(stream, isWrite, obj.handle.y);
        ser::ReadWriteStream(stream, isWrite, obj.handle.z);
        ser::ReadWriteStream(stream, isWrite, obj.handle.w);
        return stream;
    }
}