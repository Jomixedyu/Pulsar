#include <Apatite/ObjectBase.h>
#include <CoreLib/Guid.h>
#include <map>

namespace apatite
{

    static auto _object_table()
    {
        static auto table = new std::map<object_id, wptr<ObjectBase>>;
        return table;
    }

    static inline object_id _NewId()
    {
        return object_id::create_new();
    }

    bool RuntimeObjectWrapper::GetObject(object_id id, wptr<ObjectBase>* out)
    {
        auto it = _object_table()->find(id);
        if (it != _object_table()->end())
        {
            *out = it->second;
            return true;
        }
        return false;
    }

    bool RuntimeObjectWrapper::IsValid(object_id id)
    {
        if (id.is_empty()) return false;
        return _object_table()->find(id) != _object_table()->end();
    }
    void RuntimeObjectWrapper::NewInstance(sptr<ObjectBase> ptr)
    {
        auto id = _NewId();
        ptr->object_id_ = id;
        _object_table()->insert({ id, wptr<ObjectBase>(ptr) });
    }

    void RuntimeObjectWrapper::DestroyObject(const sptr<ObjectBase>& obj)
    {
        _object_table()->erase(obj->get_object_id());
    }

    void RuntimeObjectWrapper::ForceDestroyObject(object_id id)
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
        assert(!this->object_id_);
        RuntimeObjectWrapper::NewInstance(self());
        this->OnConstruct();
    }
    bool ObjectBase::IsAlive() const
    {
        return RuntimeObjectWrapper::IsValid(this->get_object_id());
    }
    void ObjectBase::OnConstruct()
    {
    }

    ObjectBase::~ObjectBase()
    {
        if (!this->object_id_.is_empty())
        {
            this->Destroy();
        }
    }
    void ObjectBase::Destroy()
    {
        this->OnDestroy();
        RuntimeObjectWrapper::ForceDestroyObject(this->object_id_);
        this->object_id_ = object_id{};
    }

    void ObjectBase::OnDestroy()
    {

    }
}