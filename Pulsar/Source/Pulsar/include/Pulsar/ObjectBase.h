#pragma once

#include <CoreLib/Core.h>
#include <CoreLib/List.h>
#include <CoreLib/Reflection.h>
#include <cstdint>
#include <string>
#include <memory>
#include <cassert>
#include <CoreLib/CommonException.h>
#include <CoreLib/Events.hpp>
#include <CoreLib/Guid.h>
#include <Pulsar/EngineMath.h>
#include <unordered_map>

#define WITH_APATITE_EDITOR

#define DECL_PTR(Class) \
CORELIB_DECL_SHORTSPTR(Class); \
using Class##_ref = ::pulsar::ObjectPtr<Class>;


namespace pulsar
{
    using namespace jxcorlib;

    CORELIB_DECL_ASSEMBLY(Pulsar);

    class EngineException : public ExceptionBase
    {
    public:
        virtual const char* name() const override { return "EngineException"; }
        using ExceptionBase::ExceptionBase;
    };

    using ObjectHandle = guid_t;

    template<typename K, typename V>
    using hash_map = std::unordered_map<K, V>;

    class ObjectBase : public Object
    {
        friend class RuntimeObjectWrapper;
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::ObjectBase, Object);
    public:
        ObjectBase();
        virtual ~ObjectBase() override;
    public:

        ObjectHandle GetObjectHandle() const { return this->m_objectHandle; }
    public:
        void Destroy();
        void Construct();
        bool IsAlive() const;
    protected:
        virtual void OnConstruct();
        virtual void OnDestroy();
    protected:
        ObjectHandle m_objectHandle;
    };
    

    struct ObjectPtrBase
    {
        ObjectHandle handle{};
        ObjectHandle GetHandle() const
        {
            return handle;
        }
    };

    class RuntimeObjectWrapper final
    {
    public:
        static ObjectBase* GetObject(ObjectHandle id);
        static sptr<ObjectBase> GetSharedObject(ObjectHandle id);
        static bool IsValid(ObjectHandle id);
        static void NewInstance(sptr<ObjectBase>&& managedObj);
        static void DestroyObject(const sptr<ObjectBase>& obj);
        static void ForceDestroyObject(ObjectHandle id);

        //<id, type, is_create>
        static Action<ObjectHandle, Type*, bool> ObjectHook;
    };


    template<typename T>
    concept baseof_objectbase = std::is_base_of<ObjectBase, T>::value;

    template<baseof_objectbase T>
    inline bool IsValid(const sptr<T>& object)
    {
        if (object == nullptr || !object->IsAlive())
        {
            return false;
        }
        return true;
    }

    template<typename T>
    struct ObjectPtr : public ObjectPtrBase
    {
        ObjectPtr() {}
        ObjectPtr(const sptr<T>& object)
        {
            if (object != nullptr)
            {
                handle = object->GetObjectHandle();
            }
        }
        sptr<T> GetShared() const
        {
            return sptr_cast<T>(RuntimeObjectWrapper::GetSharedObject(handle));
        }
        T* operator->() const
        {
            return RuntimeObjectWrapper::GetObject(handle);
        }
        bool IsValid() const
        {
            return RuntimeObjectWrapper::IsValid(handle);
        }
        void Reset()
        {
            handle = {};
        }
    };

    ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, ObjectPtrBase& obj);


    DECL_PTR(ObjectBase);
}

namespace std
{
    template<>
    struct hash<pulsar::ObjectHandle>
    {
        size_t operator()(const pulsar::ObjectHandle& handle) const noexcept
        {
            return *reinterpret_cast<const uint64_t*>(&handle) ^ *(reinterpret_cast<const uint64_t*>(&handle) + 1);
        }
    };
}