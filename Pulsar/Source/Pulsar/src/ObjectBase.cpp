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
            ObjectHandle Handle{};
            SPtr<ObjectBase> OriginalObject;
            [[not_null]] SPtr<ManagedPointer> ManagedPtr;

            [[always_inline]] bool IsValid() const
            {
                return OriginalObject != nullptr;
            }

            void Destroy()
            {
                Handle = {};
                OriginalObject.reset();
                ManagedPtr->SetEmptyPtr();
            }
        };

        struct ObjectManager
        {
            array_list<RuntimeObjectPointerValue> Array;
            hash_map<ObjectHandle, size_t> Map;

            RuntimeObjectPointerValue& Emplace(const ObjectHandle& handle)
            {
                auto& value = Array.emplace_back();
                value.Handle = handle;
                Map.emplace(handle, Array.size() - 1);
                return value;
            }

            static constexpr size_t npos = std::numeric_limits<size_t>::max();

            size_t Find(const ObjectHandle& handle) const
            {
                auto it = Map.find(handle);
                if (it != Map.end())
                    return it->second;
                return npos;
            }

            RuntimeObjectPointerValue& At(size_t index)
            {
                return Array[index];
            }

            RuntimeObjectPointerValue* Get(const ObjectHandle& handle)
            {
                auto index = Find(handle);
                if (index == npos) return nullptr;
                return &At(index);
            }

            void Remove(const ObjectHandle& handle)
            {
                auto it = Map.find(handle);
                if (it == Map.end())
                    return;

                auto index = it->second;

                auto lastHandle = Array[Array.size() - 1].Handle;

                Array[index] = Array[Array.size() - 1];
                Map[lastHandle] = index;

                Map.erase(handle);
                Array.pop_back();
            }

            void Clear()
            {
                decltype(Array){}.swap(Array);
                decltype(Map){}.swap(Map);
            }
        };
    }


    // using _object_table_t = hash_map<ObjectHandle, RuntimeObjectPointerValue>;
    // static auto& _object_table()
    // {
    //     static _object_table_t table;
    //     return table;
    // }

    static ObjectManager& GetObjectManager()
    {
        static ObjectManager Mgr;
        return Mgr;
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
        if (auto ptr = GetObjectManager().Get(id))
        {
            return ptr->OriginalObject.get();
        }
        return nullptr;
    }

    SPtr<ObjectBase> RuntimeObjectManager::GetSharedObject(const ObjectHandle& id) noexcept
    {
        if (id.is_empty())
            return nullptr;
        if (auto ptr = GetObjectManager().Get(id))
        {
            return ptr->OriginalObject;
        }
        return nullptr;
    }
    SPtr<ManagedPointer> RuntimeObjectManager::GetPointer(const ObjectHandle& id) noexcept
    {
        if (auto ptr = GetObjectManager().Get(id))
        {
            return ptr->ManagedPtr;
        }
        return nullptr;
    }

    SPtr<ManagedPointer> RuntimeObjectManager::AddWaitPointer(const ObjectHandle& id)
    {
        auto& ptr = GetObjectManager().Emplace(id);
        ptr.ManagedPtr = mksptr(new ManagedPointer{});
        return ptr.ManagedPtr;
    }

    bool RuntimeObjectManager::IsValid(const ObjectHandle& id) noexcept
    {
        if (id.is_empty())
            return false;
        if (auto ptr = GetObjectManager().Get(id))
        {
            return ptr->ManagedPtr->Pointer != nullptr;
        }
        return false;
    }

    void RuntimeObjectManager::NewInstance(SPtr<ObjectBase>&& managedObj, const ObjectHandle& handle) noexcept
    {
        const auto id = handle.is_empty() ? _NewId() : handle;
        managedObj->m_objectHandle = id;

        if (auto ptr = GetObjectManager().Get(id))
        {
            ptr->OriginalObject = std::move(managedObj);
            ptr->ManagedPtr->Pointer = ptr->OriginalObject.get();
        }
        else
        {
            auto& newPtr = GetObjectManager().Emplace(id);
            newPtr.OriginalObject = std::move(managedObj);;
            newPtr.ManagedPtr = mksptr(new ManagedPointer{});
            newPtr.ManagedPtr->Pointer = newPtr.OriginalObject.get();
        }

    }

    bool RuntimeObjectManager::DestroyObject(const ObjectHandle& id, bool isForce) noexcept
    {
        if (id.is_empty())
            return false;
        auto ptr = GetObjectManager().Get(id);
        if (ptr == nullptr) return true;

        if (const auto obj = ptr->OriginalObject.get())
        {
            const bool dontDestory = obj->HasObjectFlags(OF_LifecycleManaged);
            if (!dontDestory || (dontDestory && isForce))
            {
                obj->Destroy();
                ptr->OriginalObject.reset();
                ptr->ManagedPtr->SetEmptyPtr();
                if (ptr->ManagedPtr.use_count() == 1)
                {
                    ptr->ManagedPtr.reset();
                    GetObjectManager().Remove(id);
                }

                return true;
            }
        }
        else
        {
            if (ptr->ManagedPtr.use_count() == 1)
            {
                ptr->ManagedPtr.reset();
                GetObjectManager().Remove(id);
            }
            return true;
        }

        return false;
    }

    void RuntimeObjectManager::GetData(size_t* total, size_t* place, size_t* alive)
    {
        size_t p = 0, a = 0;
        for (auto& item : GetObjectManager().Array)
        {
            if (item.OriginalObject)
            {
                ++a;
            }
            else
            {
                if (item.ManagedPtr)
                {
                    ++p;
                }
            }
        }
        *total = GetObjectManager().Array.size();
        *place = p;
        *alive = a;
    }
    void RuntimeObjectManager::Terminate()
    {
        array_list<ObjectHandle> destroyList;
        for (const auto& item : GetObjectManager().Array)
        {
            if (item.IsValid())
            {
                if (item.OriginalObject->HasObjectFlags(OF_LifecycleManaged))
                {
                    continue;
                }
                destroyList.push_back(item.Handle);
            }
        }
        for (auto& id : destroyList)
        {
            DestroyObject(id);
        }

        // release memory
        GetObjectManager().Clear();
    }

    void RuntimeObjectManager::TickGCollect()
    {
        constexpr size_t kProcessPerFrame = 2;

        static ObjectHandle list[kProcessPerFrame];
        size_t listCount = 0;

        static size_t itIndex = -1;
        if (GetObjectManager().Array.empty())
        {
            return;
        }

        for (size_t i = 0; i < kProcessPerFrame; ++i)
        {
            ++itIndex;
            if (itIndex >= GetObjectManager().Array.size())
            {
                itIndex = 0;
            }
            auto& it = GetObjectManager().At(itIndex);
            if (!it.IsValid() && it.ManagedPtr.use_count() == 1)
            {
                list[listCount] = it.Handle;
                listCount++;
            }
        }

        for (auto i = 0; i < listCount; ++i)
        {
            GetObjectManager().Remove(list[i]);
        }

    }

    void RuntimeObjectManager::ForEachObject(const std::function<void(const RuntimeObjectInfo&)>& func)
    {
        for (auto& item : GetObjectManager().Array)
        {
            RuntimeObjectInfo info;
            info.Handle = item.Handle;
            info.Pointer = item.OriginalObject.get();
            info.ManagedCounter = item.ManagedPtr.use_count();
            func(info);
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