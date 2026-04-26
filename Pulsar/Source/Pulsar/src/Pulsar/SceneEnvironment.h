#pragma once
#include <Pulsar/EngineMath.h>
#include <ranges>

namespace pulsar
{
    class DirectionalLightSceneInfo
    {
    public:
        float    Intensity{};
        Color4f  Color{};
        Vector3f Vector{};
    };

    class SkyLightSceneInfo
    {
    public:
        float   Intensity{};
        Color4f Color{};
    };

    struct SceneRuntimeEnvironment
    {
        array_list<DirectionalLightSceneInfo*> m_directionalLights;
        array_list<SkyLightSceneInfo*>         m_skyLights;

        DirectionalLightSceneInfo* GetDirectionalLight()
        {
            DirectionalLightSceneInfo* target = nullptr;
            for (auto* dlight : m_directionalLights)
            {
                if (target == nullptr || dlight->Intensity > target->Intensity)
                    target = dlight;
            }
            return target;
        }

        SkyLightSceneInfo GetSkyLight()
        {
            SkyLightSceneInfo light{};
            for (auto* skyLight : m_skyLights)
            {
                light.Color     += skyLight->Color;
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
                m_directionalLights.erase(it);
        }
        void AddSkyLight(SkyLightSceneInfo* light)
        {
            m_skyLights.push_back(light);
        }
        void RemoveSkyLight(SkyLightSceneInfo* light)
        {
            auto it = std::ranges::find(m_skyLights, light);
            if (it != m_skyLights.end())
                m_skyLights.erase(it);
        }
    };
}
