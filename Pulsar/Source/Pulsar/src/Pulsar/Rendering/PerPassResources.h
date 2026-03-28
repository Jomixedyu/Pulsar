#pragma once
#include <gfx/GFXBuffer.h>
#include <gfx/GFXDescriptorSet.h>

#include <Pulsar/EngineMath.h>

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

    // CPU 端结构体，对应 PerPass.inc.hlsl 中的 LightRenderingData
    struct PerPassLightRenderingData
    {
        Vector3f Position;
        float    CutOff;
        Vector3f Direction;
        float    Radius;
        Vector4f Color; // w = intensity
        uint32_t Flags;
        Vector3f _Pad0;
    };

    // CPU 端结构体，对应 PerPass.inc.hlsl 中的 LightsBufferData (b2, space1)
    struct PerPassLightsBufferData
    {
        PerPassLightRenderingData Lights[63];
        uint32_t PointCount;
        uint32_t SpotCount;
        uint32_t AreaCount;
        uint32_t Flags;
        Vector4f _Pad0;
        Vector4f _Pad1;
        Vector4f _Pad2;
    };

    // 管理 set 1 (PerPass) 的 descriptor set：Camera + World + Light 合并
    class PerPassResources
    {
    public:
        void Initialize();
        void Destroy();

        void UpdateCamera(const PerPassCameraData& data);
        void UpdateWorld(const PerPassWorldData& data);
        void UpdateLights(const PerPassLightsBufferData& data);
        void Submit();

        gfx::GFXDescriptorSet_sp GetDescriptorSet() const { return m_descriptorSet; }
        gfx::GFXDescriptorSetLayout_sp GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

    private:
        gfx::GFXBuffer_sp m_cameraBuffer;
        gfx::GFXBuffer_sp m_worldBuffer;
        gfx::GFXBuffer_sp m_lightsBuffer;

        gfx::GFXDescriptorSet_sp       m_descriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_descriptorSetLayout;

        bool m_initialized = false;
    };
}
