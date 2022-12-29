#pragma once

#include <CoreLib/Core.h>
#include <CoreLib/Reflection.h>
#include <cstdint>
#include <string>
#include <memory>
#include <cassert>
#include <CoreLib/CommonException.h>
#include <CoreLib/Events.hpp>

#define WITH_APATITE_EDITOR

namespace apatite
{
    using namespace jxcorlib;

    CORELIB_DECL_ASSEMBLY(Apatite);

    class EngineException : public ExceptionBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::EngineException, ExceptionBase);
    public:
        DEF_EXCEPTION_CTOR(EngineException);
    };

    using runtime_instance_t = uint32_t;

    class ObjectBase : public Object
    {
        friend class RuntimeObjectWrapper;
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::ObjectBase, Object);
    public:
        ObjectBase();
        virtual ~ObjectBase() override;
    public:

        runtime_instance_t get_instance_id() const { return this->runtime_instance_id_; }
    public:
        void Destroy();
    protected:
        virtual void OnDestroy();
    protected:

        runtime_instance_t runtime_instance_id_;
    };
    CORELIB_DECL_SHORTSPTR(ObjectBase);


    class RuntimeObjectWrapper final
    {
    public:
        static bool GetObject(runtime_instance_t id, wptr<ObjectBase>* out);
        static bool IsValid(runtime_instance_t id);
        static runtime_instance_t NewInstance(sptr<ObjectBase> obj);
        static void DestroyObject(const sptr<ObjectBase>& obj);
        static void ForceDestroyObject(runtime_instance_t id);

        //<id, type, is_create>
        static Action<runtime_instance_t, Type*, bool> ObjectHook;
    };


    template<typename T>
    concept baseof_objectbase = std::is_base_of<ObjectBase, T>::value;

    template<baseof_objectbase T>
    inline bool IsValid(const sptr<T>& object)
    {
        if (object == nullptr || !RuntimeObjectWrapper::IsValid(object->get_instance_id()))
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
