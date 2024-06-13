#pragma once
#include <Pulsar/EngineMath.h>

namespace pulsar
{
    struct LightRenderingData
    {
        Vector3f Position;
        float CutOff;
        Vector3f Direction;
        float Radius;
        Color4f Color;
        uint32_t Flags;
        Vector3f _Pad;
    };

    struct LightsRenderingBufferData
    {
        LightRenderingData Lights[63];
        uint32_t PointCount;
        uint32_t SpotCount;
        uint32_t AreaCount;
        uint32_t Flags;
        Vector4f _Pad0;
        Vector4f _Pad1;
        Vector4f _Pad2;
    };

}