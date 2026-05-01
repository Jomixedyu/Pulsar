#pragma once
#include <gfx/GFXBuffer.h>
#include <gfx/GFXDescriptorSet.h>

#include <Pulsar/EngineMath.h>
#include <string>
#include <unordered_map>

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

    // 管理 per-pass 的共享 buffer 和按 ShaderPass 类型的 DescriptorSetLayout。
    // 每个 Pass 实例通过 AllocateSet(layout) 创建自己的 descriptor set。
    class PerPassResources
    {
    public:
        void Initialize();
        void Destroy();

        void UpdateCamera(const PerPassCameraData& data);
        void UpdateWorld(const PerPassWorldData& data);
        void UpdateLights(const PerPassLightsBufferData& data);

        // 按 ShaderPass Name 获取/创建 layout（Forward / ShadowCaster / PostProcess 等）
        gfx::GFXDescriptorSetLayout_sp GetLayout(const std::string& passName);

        // 按指定 layout 分配 descriptor set
        gfx::GFXDescriptorSet_sp AllocateSet(gfx::GFXDescriptorSetLayout_sp layout) const;

        // 按标准 binding 写入单个 buffer（调用方确保 layout 包含该 binding）
        void WriteCameraToSet(gfx::GFXDescriptorSet* set) const;
        void WriteWorldToSet(gfx::GFXDescriptorSet* set) const;
        void WriteLightsToSet(gfx::GFXDescriptorSet* set) const;

        // 便捷函数：写入全部 3 个标准 buffer（0/1/2）
        void WriteStandardBuffers(gfx::GFXDescriptorSet* set) const;

        // 按 binding 写入 texture（调用方确保 layout 包含该 binding）
        void WriteTexture(gfx::GFXDescriptorSet* set, uint32_t binding, gfx::GFXTexture2DView* view) const;

        // 提交 set 的更新
        void Submit(gfx::GFXDescriptorSet* set) const;

    private:
        gfx::GFXBuffer_sp m_cameraBuffer;
        gfx::GFXBuffer_sp m_worldBuffer;
        gfx::GFXBuffer_sp m_lightsBuffer;

        std::unordered_map<std::string, gfx::GFXDescriptorSetLayout_sp> m_layoutCache;

        bool m_initialized = false;
    };
}
