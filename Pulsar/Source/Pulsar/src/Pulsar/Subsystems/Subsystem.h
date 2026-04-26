#pragma once
#include "Assembly.h"
#include <set>

#define SUBSYSTEM_DECL(CLASS) inline struct _decl_subsystem_##CLASS { \
    _decl_subsystem_##CLASS(){ ::pulsar::SubsystemManager::RegisterSubsystem(cltypeof<CLASS>()); } } \
    _decl_subsystem_##CLASS##_;

namespace pulsar
{
    class Subsystem : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Subsystem, Object);

    public:
        virtual void OnInitializing();
        virtual void OnInitialized();
        virtual void OnTerminate();

    public:
    };
    CORELIB_DECL_SHORTSPTR(Subsystem);

    class SubsystemManager
    {
    public:
        static void RegisterSubsystem(Type* type);
        static void UnregisterSubsystem(Type* type);
        static const std::set<Type*>& GetAllSubsystems();
    };

} // namespace pulsar