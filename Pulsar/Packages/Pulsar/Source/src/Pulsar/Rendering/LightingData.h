#pragma once
#include <Pulsar/EngineMath.h>

namespace pulsar
{
    struct LightShaderParameter
    {
        Vector4f WorldPosition;
        Vector4f DirectionAndFalloffExponent;
        Vector4f Color;
        Vector2f SpotAngles;
        Vector2f SourceAndSoftSourceRadius;
    };
}