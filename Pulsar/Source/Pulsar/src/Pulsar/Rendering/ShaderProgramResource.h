#pragma once
#include <Pulsar/Rendering/ShaderPropertyLayout.h>

#include <gfx/GFXGpuProgram.h>
#include <gfx/GFXDescriptorSet.h>

#include <CoreLib/Guid.h>

#include <string>
#include <vector>
#include <functional>

namespace pulsar
{
    struct ShaderVariantKey
    {
        guid_t m_shaderGuid;
        std::string m_passName;
        std::string m_interface;
        std::vector<std::string> m_features;

        bool operator==(const ShaderVariantKey& other) const
        {
            return m_shaderGuid == other.m_shaderGuid
                && m_passName == other.m_passName
                && m_interface == other.m_interface
                && m_features == other.m_features;
        }
    };

    struct ShaderVariantKeyHash
    {
        size_t operator()(const ShaderVariantKey& key) const
        {
            size_t h = std::hash<guid_t>{}(key.m_shaderGuid);
            h ^= std::hash<std::string>{}(key.m_passName) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<std::string>{}(key.m_interface) + 0x9e3779b9 + (h << 6) + (h >> 2);
            for (const auto& f : key.m_features)
            {
                h ^= std::hash<std::string>{}(f) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return h;
        }
    };

    class ShaderProgramResource
    {
    public:
        ShaderProgramResource() = default;
        ~ShaderProgramResource() = default;

        const ShaderVariantKey& GetKey() const { return m_key; }
        const ShaderPropertyLayout& GetLayout() const { return m_layout; }

        const std::vector<gfx::GFXGpuProgram_sp>& GetGpuPrograms() const { return m_gpuPrograms; }
        const gfx::GFXDescriptorSetLayout_sp& GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

    public:
        ShaderVariantKey m_key;
        ShaderPropertyLayout m_layout;

        std::vector<gfx::GFXGpuProgram_sp> m_gpuPrograms;
        gfx::GFXDescriptorSetLayout_sp m_descriptorSetLayout;
    };
}
