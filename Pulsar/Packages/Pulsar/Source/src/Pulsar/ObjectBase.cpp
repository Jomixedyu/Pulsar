#include <CoreLib/Guid.h>
#include "ObjectBase.h"
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
            hash_map<ObjectHandle, RuntimeObjectPointerValue> Map;

            RuntimeObjectPointerValue& New(const ObjectHandle& handle, SPtr<ObjectBase>&& managedObj)
            {
                RuntimeObjectPointerValue value;
                value.Handle = handle;
                value.OriginalObject = std::move(managedObj);
                value.ManagedPtr = mksptr(new ManagedPointer);
                value.ManagedPtr->Pointer = value.OriginalObject.get();
                Map.insert({handle, value});

                return Map[handle];
            }

            static constexpr size_t npos = std::numeric_limits<size_t>::max();


            RuntimeObjectPointerValue* Get(const ObjectHandle& handle)
            {
                auto it = Map.find(handle);
                if (it != Map.end())
                {
                    return &it->second;
                }
                return nullptr;
            }

            void Remove(const ObjectHandle& handle)
            {
                auto it = Map.find(handle);
                if (it == Map.end())
                    return;

                auto& value = it->second;

                value.ManagedPtr->SetEmptyPtr();

                Map.erase(handle);
            }

            void Clear()
            {
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

    static auto& _depends()
    {
        static hash_map<ObjectHandle, array_list<ObjectHandle>> map;
        return map;
    }
    static auto& _refs()
    {
        static hash_map<ObjectHandle, array_list<ObjectHandle>> map;
        return map;
    }

    Action<ObjectBase*> RuntimeObjectManager::OnPostEditChanged{};

    static inline ObjectHandle _NewId()
    {
        static uint64_t id = 0;
        ++id;

        ObjectHandle h;
        h.value = id;
        return h;
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

    void RuntimeObjectManager::NewInstance(SPtr<ObjectBase>&& managedObj) noexcept
    {
        const auto id = _NewId();
        managedObj->m_objectHandle = id;

        GetObjectManager().New(id, std::move(managedObj));

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
                // Clean up observer/dependency relations before destroying
                {
                    auto& deps = _depends();
                    auto& refs = _refs();

                    // id was a source: remove its subscriptions to dependencies
                    auto refIt = refs.find(id);
                    if (refIt != refs.end())
                    {
                        for (auto& depHandle : refIt->second)
                        {
                            auto depIt = deps.find(depHandle);
                            if (depIt != deps.end())
                            {
                                erase(depIt->second, id);
                                if (depIt->second.empty())
                                {
                                    deps.erase(depIt);
                                }
                            }
                        }
                        refs.erase(refIt);
                    }

                    // id was a dependency: remove it from other sources' subscriptions
                    auto depIt = deps.find(id);
                    if (depIt != deps.end())
                    {
                        for (auto& srcHandle : depIt->second)
                        {
                            auto srcRefIt = refs.find(srcHandle);
                            if (srcRefIt != refs.end())
                            {
                                erase(srcRefIt->second, id);
                                if (srcRefIt->second.empty())
                                {
                                    refs.erase(srcRefIt);
                                }
                            }
                        }
                        deps.erase(depIt);
                    }
                }

                obj->Destroy(); // set object handle to empty

                ptr->OriginalObject.reset(); // destructor, release object and memory
                GetObjectManager().Remove(id);

                return true;
            }
        }

        return false;
    }

    void RuntimeObjectManager::GetData(size_t* total, size_t* place, size_t* alive)
    {
        size_t p = 0, a = 0;
        for (auto& item : GetObjectManager().Map)
        {
            auto& value = item.second;
            if (value.OriginalObject)
            {
                ++a;
            }
            else
            {
                if (value.ManagedPtr)
                {
                    ++p;
                }
            }
        }
        *total = GetObjectManager().Map.size();
        *place = p;
        *alive = a;
    }
    void RuntimeObjectManager::Terminate()
    {
        array_list<ObjectHandle> destroyList;
        for (auto& item : GetObjectManager().Map)
        {
            auto& value = item.second;
            if (value.IsValid())
            {
                if (value.OriginalObject->HasObjectFlags(OF_LifecycleManaged))
                {
                    continue;
                }
                destroyList.push_back(value.Handle);
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

    }

    void RuntimeObjectManager::ForEachObject(const std::function<void(const RuntimeObjectInfo&)>& func)
    {
        for (auto& item : GetObjectManager().Map)
        {
            auto& value = item.second;
            RuntimeObjectInfo info;
            info.Handle = value.Handle;
            info.Pointer = value.OriginalObject.get();
            info.ManagedCounter = value.ManagedPtr.use_count();
            func(info);
        }
    }

    void RuntimeObjectManager::NotifyDependencySource(ObjectHandle dest, DependencyObjectState id)
    {
        auto it = _depends().find(dest);
        if (it != _depends().end())
        {
            for (const auto& srcId : it->second)
            {
                if (ObjectPtrBase src = srcId)
                {
                    src->OnNotifyObserver(dest, id);
                }
            }
        }
    }

    void RuntimeObjectManager::RebuildMessageBox(ObjectBase* obj)
    {
        auto& deps = _depends();
        auto& refs = _refs();

        auto sourceHandle = obj->GetObjectHandle();

        auto it = refs.find(sourceHandle);
        if (it != refs.end())
        {
            for (auto& ref : it->second)
            {
                auto depIt = deps.find(ref);
                if (depIt != deps.end())
                {
                    erase(depIt->second, sourceHandle);
                    if (depIt->second.empty())
                    {
                        deps.erase(depIt);
                    }
                }
            }
            refs.erase(it);
        }

        array_list<ObjectHandle> dependencies;
        obj->GetSubscribeObserverHandles(dependencies);

        if (!dependencies.empty())
        {
            refs[sourceHandle] = dependencies;
            for (auto& dependency : dependencies)
            {
                deps[dependency].push_back(sourceHandle);
            }
        }
    }

    ObjectBase::ObjectBase()
    {
    }
    void ObjectBase::Construct()
    {
        assert(this);
        assert(!this->m_objectHandle);
        RuntimeObjectManager::NewInstance(self());
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

    void ObjectBase::OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg)
    {
    }

    void ObjectBase::RebuildObserver()
    {
        RuntimeObjectManager::RebuildMessageBox(this);
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
        RuntimeObjectManager::NotifyDependencySource(GetObjectHandle(), msg);
    }


} // namespace pulsar