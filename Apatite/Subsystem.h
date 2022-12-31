#pragma once
#include "ObjectBase.h"

#define APATITE_SUBSYSTEM() struct __apatite_subsystem { __apatite_subsystem(){ ::apatite::__ApatiteSubsystemRegistry::types()->push_back(StaticType()); } } __apatite_subsystem_;

namespace apatite
{
    class Subsystem : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Subsystem, Object);
    public:
        virtual void OnInitializing() {}
        virtual void OnInitialized() {}
        virtual void OnTerminate() {}
    public:

    };
    CORELIB_DECL_SHORTSPTR(Subsystem);

    class __ApatiteSubsystemRegistry
    {
    public:
        static array_list<Type*>* types();
        
    };
}