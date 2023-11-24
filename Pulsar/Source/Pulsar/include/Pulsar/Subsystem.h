#pragma once
#include "ObjectBase.h"

#define APATITE_SUBSYSTEM() struct __pulsar_subsystem { __pulsar_subsystem(){ ::pulsar::__PulsarSubsystemRegistry::types()->push_back(StaticType()); } } __pulsar_subsystem_;

namespace pulsar
{
    class Subsystem : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Subsystem, Object);
    public:
        virtual void OnInitializing() {}
        virtual void OnInitialized() {}
        virtual void OnTerminate() {}
    public:

    };
    CORELIB_DECL_SHORTSPTR(Subsystem);

    class __PulsarSubsystemRegistry
    {
    public:
        static array_list<Type*>* types();
        
    };
}