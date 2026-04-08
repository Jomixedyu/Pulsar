#pragma once

#include "Prefab.h"

#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/CubeMap.h>
#include <Pulsar/Assets/NodeCollection.h>
#include <Pulsar/Node.h>
#include <Pulsar/ObjectBase.h>
#include <vector>

namespace pulsar
{
    class Node;
    class World;

    class Scene : public NodeCollection
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Scene, NodeCollection);
        DECL_OBJECTPTR_SELF
    public:
        Scene();
        virtual ~Scene() override = default;

        void Serialize(AssetSerializer* s) override;
        void OnInstantiateAsset(AssetObject* obj) override;

        // Scene 特有：直射光/SkyLight 注册
        void OnAddNode(ObjectPtr<Node> node) override;
        void OnRemoveNode(ObjectPtr<Node> node) override;

        static RCPtr<Scene> StaticCreate(string_view name);

        SceneRuntimeEnvironment* GetRuntimeEnvironment() override { return &m_runtimeEnvironment; }

    protected:
        void OnDestroy() override;
    private:
        SceneRuntimeEnvironment m_runtimeEnvironment{};

        CORELIB_REFL_DECL_FIELD(m_cubemap);
        CubeMapAsset_ref m_cubemap;
    };
    DECL_PTR(Scene);
}

