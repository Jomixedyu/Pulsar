#pragma once

#include <CoreLib/Core.h>
#include <CoreLib/Reflection.h>
#include <cstdint>
#include <string>
#include <memory>
#include <cassert>
#include <CoreLib/CommonException.h>
#include <CoreLib/Events.hpp>
#include <CoreLib/Guid.h>
#include <Pulsar/Math.h>

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

    using object_id = guid_t;

    class ObjectBase : public Object
    {
        friend class RuntimeObjectWrapper;
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::ObjectBase, Object);
    public:
        ObjectBase();
        virtual ~ObjectBase() override;
    public:

        object_id get_object_id() const { return this->object_id_; }
    public:
        void Destroy();
        void Construct();
        bool IsAlive() const;
    protected:
        virtual void OnConstruct();
        virtual void OnDestroy();
    protected:
        object_id object_id_;
    };
    CORELIB_DECL_SHORTSPTR(ObjectBase);


    class RuntimeObjectWrapper final
    {
    public:
        static bool GetObject(object_id id, wptr<ObjectBase>* out);
        static bool IsValid(object_id id);
        static void NewInstance(sptr<ObjectBase> obj);
        static void DestroyObject(const sptr<ObjectBase>& obj);
        static void ForceDestroyObject(object_id id);

        //<id, type, is_create>
        static Action<object_id, Type*, bool> ObjectHook;
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
}
