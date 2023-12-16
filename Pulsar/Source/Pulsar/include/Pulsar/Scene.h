#pragma once

#include <vector>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/Node.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/CubeMap.h>
#include <Pulsar/Assets/NodeCollection.h>

namespace pulsar
{
    class DirectionalLightSceneInfo
    {
    public:
        float Intensity;
        Color4f Color;
        Vector3f Vector;
    };

    struct SceneRuntimeEnvironment
    {
    public:
        DirectionalLightSceneInfo* DirectionalLight;
    };

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
        virtual void Tick(Ticker ticker);

        virtual void OnAddNode(Node_ref node) override;
        virtual void OnRemoveNode(Node_ref node) override;

        static ObjectPtr<Scene> StaticCreate(string_view name);

        World* GetWorld() const { return m_runtimeWorld; }

        SceneRuntimeEnvironment& GetRuntimeEnvironment() { return m_runtimeEnvironment; }

        void AddDirectionalLight(DirectionalLightSceneInfo* light);
        void RemoveDirectionalLight(DirectionalLightSceneInfo* light);
        void UpdateDirectionalLight();
    protected:
        void OnDestroy() override;
    private:
        SceneRuntimeEnvironment m_runtimeEnvironment{};

        CORELIB_REFL_DECL_FIELD(cubemap_);
        CubeMapAsset_ref cubemap_;

        World* m_runtimeWorld = nullptr;


        array_list<DirectionalLightSceneInfo*> m_directionalLights;
    };
    DECL_PTR(Scene);
}

