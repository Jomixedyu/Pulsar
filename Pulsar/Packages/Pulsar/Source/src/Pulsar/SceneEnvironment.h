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

    struct SceneRuntimeEnvironment
    {
        array_list<DirectionalLightSceneInfo*> m_directionalLights;

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
    };
}
