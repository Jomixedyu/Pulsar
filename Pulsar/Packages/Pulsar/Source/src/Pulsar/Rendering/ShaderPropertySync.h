#pragma once
#include <Pulsar/Rendering/ShaderPropertySheet.h>
#include <Pulsar/Rendering/ShaderPropertyLayout.h>
#include <Pulsar/Rendering/ShaderPropertyRenderData.h>
#include <Pulsar/Rendering/RenderResourceRegistry.h>

#include <gfx/GFXBuffer.h>
#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXTexture.h>

#include <vector>

namespace pulsar
{
    class ShaderPropertySync
    {
    public:
        // 【游戏线程】将 Sheet 解析成渲染线程专用快照：纹理解析成 GPU 句柄
        // （确保 CreateGPUResource 在游戏线程触发），常量拷贝成纯值。与 layout 无关。
        // 调用前请确保 sheet 已通过 ApplyShaderDefaults 填入默认值。
        static ShaderPropertyRenderData BuildRenderData(const ShaderPropertySheet& sheet);

        // 【渲染线程】把常量快照打包上传到共享的 PerMaterial cbuffer（材质唯一一份）。
        // 只消费 POD 字节；每次参数变更调用一次即可，无需按变体重复上传。
        static void UploadPerMaterialCBuffer(
            const ShaderPropertyRenderData& data,
            const ShaderLayout& layout,
            gfx::GFXBuffer* cbuffer);

        // 【渲染线程】按 set0 反射把共享 cbuffer + 已解析纹理 view 填入 registry（缺失项留空，由
        // assembler 回落到内建兜底）。返回持有纹理 view 强引用的容器，须存活到 registry 使用完毕
        // （registry 内存裸指针）。供 DescriptorSetCache 先据此算 key 再装配。
        static std::vector<gfx::GFXTexture2DView_sp> BuildSet0Registry(
            const ShaderPropertyRenderData& data,
            const ShaderPropertySetLayout& set0,
            gfx::GFXBuffer* cbuffer,
            RenderResourceRegistry& reg);

    private:
        static void WritePropertyToBuffer(uint8_t* buffer, const BufferMember& entry, const ShaderPropertyValue& prop);
    };
}
