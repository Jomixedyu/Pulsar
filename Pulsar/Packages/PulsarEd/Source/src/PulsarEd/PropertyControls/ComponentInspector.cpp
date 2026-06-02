#include "PropertyControls/ComponentInspector.h"
#include <Pulsar/Components/Component.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>

namespace pulsared
{
    static auto& manager()
    {
        static hash_map<Type*, ComponentInspector*> inst;
        return inst;
    }

    void ComponentInspectorManager::RegisterInspector(Type* type, ComponentInspector* inspector)
    {
        manager()[type] = inspector;
    }

    ComponentInspector* ComponentInspectorManager::FindInspector(Type* type)
    {
        auto& mgr = manager();
        auto it = mgr.find(type);
        if (it != mgr.end())
            return it->second;
        for (auto& [k, v] : mgr)
        {
            if (type->IsSubclassOf(k))
                return v;
        }
        return nullptr;
    }

    void ComponentInspector::OnDrawImGui(pulsar::Component* comp, bool showDebug)
    {
        PImGui::ObjectFieldProperties(
            comp->GetType(), comp->GetType(),
            comp, comp, showDebug);
    }
}
