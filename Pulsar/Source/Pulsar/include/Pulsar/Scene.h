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
    class DirectionalLightSceneInfo
    {
    public:
        float Intensity{};
        Color4f Color{};
        Vector3f Vector{};
    };
    class SkyLightSceneInfo
    {
    public:
        float Intensity{};
        Color4f Color;
    };

    struct SceneRuntimeEnvironment
    {
        array_list<DirectionalLightSceneInfo*> m_directionalLights;
        array_list<SkyLightSceneInfo*> m_skyLights;

    public:
        DirectionalLightSceneInfo* GetDirectionalLight()
        {
            DirectionalLightSceneInfo* target = nullptr;
            for (auto dlight : m_directionalLights)
            {
                if(target == nullptr || dlight->Intensity > target->Intensity)
                {
                    target = dlight;
                }
            }
            return target;
        }
        SkyLightSceneInfo GetSkyLight()
        {
            SkyLightSceneInfo light{};
            for (auto skyLight : m_skyLights)
            {
                light.Color += skyLight->Color;
                light.Intensity += skyLight->Intensity;
            }
            return light;
        }

        void AddDirectionalLight(DirectionalLightSceneInfo* light)
        {
            m_directionalLights.push_back(light);
        }
        void RemoveDirectionalLight(DirectionalLightSceneInfo* light)
        {
            auto it = std::ranges::find(m_directionalLights, light);
            if (it != m_directionalLights.end())
            {
                m_directionalLights.erase(it);
            }
        }
        void AddSkyLight(SkyLightSceneInfo* light)
        {
            m_skyLights.push_back(light);
        }
        void RemoveSkylLight(SkyLightSceneInfo* light)
        {
            auto it = std::ranges::find(m_skyLights, light);
            if (it != m_skyLights.end())
            {
                m_skyLights.erase(it);
            }
        }
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

#ifdef WITH_EDITOR
        void AddPrefab(RCPtr<Prefab> prefab);
#endif

    protected:
        void OnDestroy() override;
    private:
        SceneRuntimeEnvironment m_runtimeEnvironment{};

        CORELIB_REFL_DECL_FIELD(cubemap_);
        CubeMapAsset_ref cubemap_;

        World* m_runtimeWorld = nullptr;


        array_list<DirectionalLightSceneInfo*> m_directionalLights;
        array_list<SkyLightSceneInfo*> m_skyLights;

        //hash_map<Prefab_ref, Prefab_ref> m_prefabInstances; // <instance, asset>
    };
    DECL_PTR(Scene);
}

