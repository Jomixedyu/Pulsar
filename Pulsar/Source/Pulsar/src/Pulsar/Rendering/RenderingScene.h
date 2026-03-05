#pragma once
#include "RenderObject.h"

namespace pulsar::rendering
{
    struct RenderingSceneConstantBuffer
    {
        Vector4f WorldSpaceLightVector;
        Vector4f WorldSpaceLightColor; // w is intensity
        Vector4f SkyLightColor; // w is intensity
        float TotalTime;
        float DeltaTime;
        uint32_t LightParameterCount;
        float _Padding0;
    };

    class RenderingScene
    {
    public:

    };

}