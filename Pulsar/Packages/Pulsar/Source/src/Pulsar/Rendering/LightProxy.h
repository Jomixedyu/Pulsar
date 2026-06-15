#pragma once
#include <Pulsar/EngineMath.h>
#include "LightingData.h"
#include "RenderProxy.h"

namespace pulsar
{
    // Render-thread mirror of a point/spot light. Holds a per-frame snapshot of the
    // game-side LightShaderParameter, written via the update queue on the game thread
    // and read on the render thread. One per PointLightComponent.
    class PointLightProxy : public rendering::RenderProxy
    {
    public:
        LightShaderParameter Param{};
    };

    // Render-thread mirror of a directional light. The renderer picks the brightest
    // proxy among all registered directional lights. One per DirectionalLightComponent.
    class DirectionalLightProxy : public rendering::RenderProxy
    {
    public:
        Vector3f Vector{};
        Color4f  Color{};
        float    Intensity = 0.f;
    };
}
