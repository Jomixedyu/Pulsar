#pragma once
#include <Pulsar/Rendering/Types.h>

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
    };

    struct TextureEntry
    {
        std::string m_name;
        uint32_t m_bindingPoint{};
    };

    struct ShaderPropertyLayout
    {
        std::vector<CBufferEntry> m_constantEntries;
        std::vector<TextureEntry> m_textureEntries;
        uint32_t m_totalCBufferSize{};
    };
}
