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
        List_rsp<Node_sp> GetRootNodes() const { return this->scene_nodes_; }
        List_sp<Node_sp> GetRootNodes() { return this->scene_nodes_; }

        Scene();
        virtual ~Scene() override
        {
            int a = 3;
        }
    public:
        void AddNode(Node_rsp node);
        void RemoveNode(Node_rsp node);

        static sptr<Scene> StaticCreate(string_view name);
    private:

        List_sp<Node_sp> scene_nodes_;

        CORELIB_REFL_DECL_FIELD(cubemap_);
        CubeMapAsset_sp cubemap_;


    };
    DECL_PTR(Scene);
}

