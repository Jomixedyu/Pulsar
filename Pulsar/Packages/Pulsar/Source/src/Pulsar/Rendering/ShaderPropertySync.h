#pragma once
#include <Pulsar/Rendering/ShaderPropertySheet.h>
#include <Pulsar/Rendering/ShaderPropertyLayout.h>

#include <gfx/GFXBuffer.h>
#include <gfx/GFXDescriptorSet.h>

namespace pulsar
{
    class ShaderPropertySync
    {
    public:
        // 将 Sheet 中的值按 Layout 同步到 GFXBuffer (cbuffer) 和 GFXDescriptorSet
        // 调用前请确保 sheet 已通过 ApplyShaderDefaults 填入默认值
        static void SyncSheetToGpu(
            const ShaderPropertySheet& sheet,
            const ShaderPropertyLayout& layout,
            gfx::GFXBuffer* cbuffer,
            gfx::GFXDescriptorSet* descriptorSet);

    private:
        static void SyncConstants(
            const ShaderPropertySheet& sheet,
            const ShaderPropertyLayout& layout,
            gfx::GFXBuffer* cbuffer);

        static void SyncTextures(
            const ShaderPropertySheet& sheet,
            const ShaderPropertyLayout& layout,
            gfx::GFXDescriptorSet* descriptorSet);

        static void WritePropertyToBuffer(uint8_t* buffer, const CBufferEntry& entry, const ShaderPropertyValue& prop);
    };
}
