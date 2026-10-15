#pragma once
#include <Pulsar/EngineMath.h>
#include <cstdint>

#include "LightingData.h"

namespace pulsar
{
    // CPU 端结构体，对应 PerPass.inc.hlsl 中的 CameraData (b0, space1)
    struct PerPassCameraData
    {
        Matrix4f MatrixV;
        Matrix4f InvMatrixV;
        Matrix4f MatrixP;
        Matrix4f InvMatrixP;
        Matrix4f MatrixVP;
        Matrix4f InvMatrixVP;
        Vector4f CamPosition;
        float    CamNear;
        float    CamFar;
        Vector2f Resolution;
        Vector4f _Padding1;
        Vector4f _Padding2;
        Matrix4f _Padding3;
    };

    // CPU 端结构体，对应 PerPass.inc.hlsl 中的 WorldData (b1, space1)
    struct PerPassWorldData
    {
        Vector4f WorldSpaceLightVector;
        Vector4f WorldSpaceLightColor;
        Vector4f SkyLightColor;
        float    TotalTime;
        float    DeltaTime;
        uint32_t LightParameterCount;
        float    _Padding0;
    };

    // CPU 端结构体，对应 PerPass.inc.hlsl 中的 LightsBufferData (b2, space1)
    struct PerPassLightsBufferData
    {
        LightShaderParameter Lights[64];
    };
}
