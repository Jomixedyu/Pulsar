#pragma once
#include <Pulsar/Rendering/ShaderPropertySheet.h>
#include <Pulsar/Rendering/ShaderPropertyLayout.h>
#include <Pulsar/Rendering/ShaderPropertyRenderData.h>

#include <gfx/GFXBuffer.h>
#include <gfx/GFXDescriptorSet.h>

namespace pulsar
{
    class ShaderPropertySync
    {
    public:
        // 【游戏线程】将 Sheet 解析成渲染线程专用快照：纹理解析成 GPU 句柄
        // （确保 CreateGPUResource 在游戏线程触发），常量拷贝成纯值。与 layout 无关。
        // 调用前请确保 sheet 已通过 ApplyShaderDefaults 填入默认值。
        static ShaderPropertyRenderData BuildRenderData(const ShaderPropertySheet& sheet);

        // 【渲染线程】把已解析的快照写入 GFXBuffer (cbuffer) 和 GFXDescriptorSet。
        // 只消费 POD 句柄与字节，不访问任何资产。
        static void ApplyRenderData(
            const ShaderPropertyRenderData& data,
            const ShaderLayout& layout,
            gfx::GFXBuffer* cbuffer,
            gfx::GFXDescriptorSet* descriptorSet);

    private:
        static void WritePropertyToBuffer(uint8_t* buffer, const BufferMember& entry, const ShaderPropertyValue& prop);
    };
}
