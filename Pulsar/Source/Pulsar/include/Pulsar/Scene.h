#pragma once

#include <vector>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/Node.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/CubeMap.h>

namespace pulsar
{

    class Node;

    class Scene : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::Scene, AssetObject)
    public:
        List_sp<Node_ref> GetRootNodes() { return m_sceneNodes; }

        Scene();
        virtual ~Scene() override
        {
        }
    public:
        void AddNode(Node_ref node);
        void RemoveNode(Node_ref node);

        static ObjectPtr<Scene> StaticCreate(string_view name);
    protected:
        virtual void OnDestroy() override;
    private:

        List_sp<Node_ref> m_sceneNodes;

        CORELIB_REFL_DECL_FIELD(cubemap_);
        CubeMapAsset_ref cubemap_;


    };
    DECL_PTR(Scene);
}

