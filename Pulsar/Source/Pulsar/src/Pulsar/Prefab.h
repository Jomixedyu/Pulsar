#pragma once
#include "Assets/NodeCollection.h"

namespace pulsar
{
    class Prefab : public NodeCollection
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Prefab, NodeCollection);
        CORELIB_CLASS_ATTR(new AssetIconAttribute("Editor/Icons/prefab.png"))
    public:
        static RCPtr<Prefab> StaticCreate(string_view name);

        SceneRuntimeEnvironment* GetRuntimeEnvironment() override { return &m_runtimeEnvironment; }
    private:
        SceneRuntimeEnvironment m_runtimeEnvironment{};
    };
    DECL_PTR(Prefab);

}