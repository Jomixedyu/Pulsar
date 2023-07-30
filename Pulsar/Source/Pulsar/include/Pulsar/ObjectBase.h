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

#define WITH_APATITE_EDITOR

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
    CORELIB_DECL_SHORTSPTR(ObjectBase);


    class RuntimeObjectWrapper final
    {
    public:
        static bool GetObject(ObjectHandle id, wptr<ObjectBase>* out);
        static bool IsValid(ObjectHandle id);
        static void NewInstance(sptr<ObjectBase> obj);
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

    template<baseof_objectbase T>
    inline void ResetIfInvalid(sptr<T>& object)
    {
        if (!IsValid(object)) object = nullptr;
    }

    struct ObjectPtrBase
    {
        ObjectHandle handle{};

        ObjectHandle GetHandle() const
        {
            return handle;
        }
    };

    template<typename T>
    struct ObjectPtr : public ObjectPtrBase
    {
    private:
        sptr<T> m_ptr;
    public:
        ObjectPtr() {}
        ObjectPtr(const sptr<T>& object)
        {
            if (object != nullptr)
            {
                m_ptr = object;
                handle = object->GetObjectHandle();
            }
        }

        bool IsValid() const { return m_ptr != nullptr; }
        sptr<T> Get() const { return m_ptr; }
        T* GetPtr() const { return m_ptr.get(); }
        sptr<T>& operator->() { return m_ptr; }

        void Reset()
        {
            handle = {};
        }
    };

    template<typename T>
    struct WeakPtr : public ObjectPtrBase
    {
        WeakPtr() {}
        WeakPtr(const sptr<T>& object)
        {
            if (object != nullptr)
            {
                handle = object->GetObjectHandle();
            }
        }
        sptr<T> Get() const
        {
            ObjectBase_wp obj;
            if (RuntimeObjectWrapper::GetObject(handle, &obj))
            {
                return sptr_cast<T>(obj.lock());
            }
            return nullptr;
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

}
