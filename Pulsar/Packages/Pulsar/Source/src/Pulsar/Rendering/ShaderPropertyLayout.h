#pragma once
#include <Pulsar/Rendering/Types.h>
#include <gfx/GFXGpuProgram.h>
#include <gfx/GFXDescriptorSet.h>
#include <gfx/TextureClasses.h>

#include <string>
#include <vector>
#include <cstdint>

namespace pulsar
{
    // A single member inside a buffer block (offset/size/type for CPU-side packing).
    struct BufferMember
    {
        std::string m_name;
        uint32_t m_offset{};
        uint32_t m_size{};
        ShaderPropertyType m_type{};
    };

    // A single reflected descriptor binding within a set. Covers every descriptor
    // kind (constant buffer / structured buffer / texture / sampler ...); m_type
    // discriminates. m_size / m_members are only meaningful for buffer kinds.
    struct DescriptorBinding
    {
        std::string m_name;
        uint32_t m_bindingPoint{};
        gfx::GFXDescriptorType m_type{};
        gfx::GFXGpuProgramStageFlags m_stageFlags = gfx::GFXGpuProgramStageFlags::None;
        uint32_t m_size{};                    // buffer byte size; 0 for textures/samplers
        std::vector<BufferMember> m_members;  // buffer members; empty for textures/samplers

        // Image view dimensionality for texture bindings; used to pick the matching
        // fallback view when a texture is missing. None for buffers/samplers.
        gfx::GFXTextureDataType m_viewDimension = gfx::GFXTextureDataType::None;

        bool IsBuffer() const
        {
            return m_type == gfx::GFXDescriptorType::ConstantBuffer
                || m_type == gfx::GFXDescriptorType::ConstantBufferDynamic
                || m_type == gfx::GFXDescriptorType::StructuredBuffer;
        }
    };

    // Reflected layout of a single descriptor set
    // (set0 perMaterial / set1 perPass / set2 perDraw).
    struct ShaderPropertySetLayout
    {
        std::vector<DescriptorBinding> m_bindings;
    };

    // Full reflected layout across every descriptor set, indexed by set number.
    // Sets are contiguous; a set with no bindings is an empty bucket.
    struct ShaderLayout
    {
        std::vector<ShaderPropertySetLayout> m_sets;

        const ShaderPropertySetLayout* FindSet(uint32_t set) const
        {
            return set < m_sets.size() ? &m_sets[set] : nullptr;
        }

        ShaderPropertySetLayout& FindOrAddSet(uint32_t set)
        {
            if (set >= m_sets.size())
                m_sets.resize(set + 1);
            return m_sets[set];
        }
    };
}

