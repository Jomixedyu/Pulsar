#include "psc/ShaderReflect.h"
#include "../spirv-cross/spirv_cross.hpp"

#include <cstring>

namespace psc
{
    ReflectedShaderResources ReflectSpirvResources(const std::vector<char>& spirvData)
    {
        ReflectedShaderResources result{};

        // spirv-cross 需要 uint32_t 数组
        std::vector<uint32_t> spirvWords(spirvData.size() / sizeof(uint32_t));
        std::memcpy(spirvWords.data(), spirvData.data(), spirvData.size());

        spirv_cross::Compiler compiler(std::move(spirvWords));
        auto resources = compiler.get_shader_resources();

        // Uniform Buffers (cbuffer)
        for (auto& ub : resources.uniform_buffers)
        {
            ReflectedUniformBuffer reflected{};
            reflected.Name = compiler.get_name(ub.id);
            reflected.Set = compiler.get_decoration(ub.id, spv::DecorationDescriptorSet);
            reflected.Binding = compiler.get_decoration(ub.id, spv::DecorationBinding);

            auto& type = compiler.get_type(ub.type_id);
            reflected.Size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));

            for (uint32_t i = 0; i < type.member_types.size(); i++)
            {
                ReflectedCBufferMember member{};
                member.Name = compiler.get_member_name(ub.base_type_id, i);
                member.Offset = compiler.type_struct_member_offset(type, i);
                member.Size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(type, i));
                reflected.Members.push_back(std::move(member));
            }

            result.UniformBuffers.push_back(std::move(reflected));
        }

        // Sampled Images (texture + sampler)
        for (auto& img : resources.sampled_images)
        {
            ReflectedTexture reflected{};
            reflected.Name = compiler.get_name(img.id);
            reflected.Set = compiler.get_decoration(img.id, spv::DecorationDescriptorSet);
            reflected.Binding = compiler.get_decoration(img.id, spv::DecorationBinding);
            result.SampledImages.push_back(std::move(reflected));
        }

        // Separate images (texture without sampler)
        for (auto& img : resources.separate_images)
        {
            ReflectedTexture reflected{};
            reflected.Name = compiler.get_name(img.id);
            reflected.Set = compiler.get_decoration(img.id, spv::DecorationDescriptorSet);
            reflected.Binding = compiler.get_decoration(img.id, spv::DecorationBinding);
            result.SampledImages.push_back(std::move(reflected));
        }

        return result;
    }
}
