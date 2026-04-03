#pragma once
#include <Pulsar/Rendering/Types.h>
#include <gfx/GFXGpuProgram.h>

#include <string>
#include <vector>
#include <cstdint>

namespace pulsar
{
    struct CBufferEntry
    {
        std::string m_name;
        uint32_t m_offset{};
        uint32_t m_size{};
        ShaderPropertyType m_type{};
        gfx::GFXGpuProgramStageFlags m_stageFlags = gfx::GFXGpuProgramStageFlags::VertexFragment;
    };

    struct TextureEntry
    {
        std::string m_name;
        uint32_t m_bindingPoint{};
        bool m_isCombinedImageSampler = true;
        gfx::GFXGpuProgramStageFlags m_stageFlags = gfx::GFXGpuProgramStageFlags::VertexFragment;
    };

    struct ShaderPropertyLayout
    {
        std::vector<CBufferEntry> m_constantEntries;
        std::vector<TextureEntry> m_textureEntries;
        uint32_t m_totalCBufferSize{};
    };
}
