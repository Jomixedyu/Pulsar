#pragma once

#include <vector>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/Node.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/CubeMap.h>
#include <Pulsar/Assets/NodeCollection.h>

namespace pulsar
{

    class Node;
    class World;

    class Scene : public NodeCollection
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Scene, NodeCollection);
        inline ObjectPtr<Scene> self_ref() const { return this->GetObjectHandle(); }
    public:

        Scene();
        virtual ~Scene() override
        {
        }
    public:
        void BeginScene(World* world);
        void EndScene();

        virtual void OnAddNode(Node_ref node) override;
        virtual void OnRemoveNode(Node_ref node) override;

        static ObjectPtr<Scene> StaticCreate(string_view name);

        World* GetWorld() const { return m_runtimeWorld; }


    protected:
        virtual void OnDestroy() override;
    private:

        CORELIB_REFL_DECL_FIELD(cubemap_);
        CubeMapAsset_ref cubemap_;


        bool m_isRuntimeScene;
        World* m_runtimeWorld;


    };
    DECL_PTR(Scene);
}

