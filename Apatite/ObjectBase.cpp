#include <Apatite/ObjectBase.h>
#include <CoreLib/Guid.h>
#include <map>

namespace apatite
{

    static inline std::map<runtime_instance_t, wptr<ObjectBase>> _object_table;
    static runtime_instance_t _current;

    static runtime_instance_t _NewId()
    {
        return ++_current;
    }

    bool RuntimeObjectWrapper::GetObject(runtime_instance_t id, wptr<ObjectBase>* out)
    {
        auto it = _object_table.find(id);
        if (it != _object_table.end())
        {
            *out = it->second;
            return true;
        }
        return false;
    }

    bool RuntimeObjectWrapper::IsValid(runtime_instance_t id)
    {
        return _object_table.find(id) != _object_table.end();
    }
    runtime_instance_t RuntimeObjectWrapper::NewInstance(sptr<ObjectBase> ptr)
    {
        auto id = _NewId();
        ptr->runtime_instance_id_ = id;
        _object_table.insert({ id, wptr<ObjectBase>(ptr) });
        return id;
    }

    void RuntimeObjectWrapper::DestroyObject(const sptr<ObjectBase>& obj)
    {
        _object_table.erase(obj->runtime_instance_id_);
    }

    void RuntimeObjectWrapper::ForceDestroyObject(runtime_instance_t id)
    {
        _object_table.erase(id);
    }

    ObjectBase::ObjectBase()
    {
        this->runtime_instance_id_ = RuntimeObjectWrapper::NewInstance(self());
    }

    ObjectBase::~ObjectBase()
    {
        RuntimeObjectWrapper::ForceDestroyObject(this->runtime_instance_id_);
    }
}