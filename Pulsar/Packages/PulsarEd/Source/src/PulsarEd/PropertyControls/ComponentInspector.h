#pragma once
#include <PulsarEd/Assembly.h>

#define EDITOR_IMPL_COMPONENT_INSPECTOR(COMP_TYPE, INST) \
static inline struct __component_inspector_init { \
    __component_inspector_init() { ComponentInspectorManager::RegisterInspector(cltypeof<COMP_TYPE>(), INST); } \
} __component_inspector_init_; \
public: virtual Type* GetComponentType() override { \
    return cltypeof<COMP_TYPE>(); \
}

namespace pulsared
{
    class ComponentInspector
    {
    public:
        virtual ~ComponentInspector() = default;
        virtual void OnDrawImGui(pulsar::Component* comp, bool showDebug);
        virtual Type* GetComponentType() = 0;
    };

    class ComponentInspectorManager
    {
    public:
        static void RegisterInspector(Type* type, ComponentInspector* inspector);
        static ComponentInspector* FindInspector(Type* type);
    };
}
