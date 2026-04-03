#include "psc/ShaderReflect.h"
#include "../spirv-cross/spirv_cross.hpp"

#include <cstring>
#include <unordered_set>
#include <string>

namespace psc
{
    ReflectedShaderResources ReflectSpirvResources(const std::vector<char>& spirvData)
    {
        ReflectedShaderResources result{};

        // spirv-cross 需要 uint32_t 数组
        std::vector<uint32_t> spirvWords(spirvData.size() / sizeof(uint32_t));
        std::memcpy(spirvWords.data(), spirvData.data(), spirvData.size());

        spirv_cross::Compiler compiler(std::move(spirvWords));
        // Only reflect resources that are actually used (active) in this stage.
        auto activeVars = compiler.get_active_interface_variables();
        auto resources = compiler.get_shader_resources(activeVars);

        // Derive ShaderStageFlags from the SPIR-V execution model.
        ShaderStageFlags stageFlag = ShaderStageFlags::None;
        switch (compiler.get_execution_model())
        {
        case spv::ExecutionModelVertex:                 stageFlag = ShaderStageFlags::Vertex;   break;
        case spv::ExecutionModelFragment:               stageFlag = ShaderStageFlags::Fragment; break;
        case spv::ExecutionModelGLCompute:              stageFlag = ShaderStageFlags::Compute;  break;
        default:                                        stageFlag = ShaderStageFlags::All;      break;
        }

        // Uniform Buffers (cbuffer)
        for (auto& ub : resources.uniform_buffers)
        {
            ReflectedUniformBuffer reflected{};
            reflected.Name = compiler.get_name(ub.id);
            reflected.Set = compiler.get_decoration(ub.id, spv::DecorationDescriptorSet);
            reflected.Binding = compiler.get_decoration(ub.id, spv::DecorationBinding);
            reflected.StageFlags = stageFlag;

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

        // Build a set of sampler names from separate_samplers for pairing detection.
        // HLSL Texture2D + SamplerState compile to separate_images + separate_samplers in SPIR-V.
        // We detect "combined" intent by the naming convention: "Sampler_<TextureName>".
        std::unordered_set<std::string> pairedSamplerTexNames;
        for (auto& smp : resources.separate_samplers)
        {
            std::string sName = compiler.get_name(smp.id);
            // Convention: SamplerState Sampler__BaseColorMap  →  paired with _BaseColorMap
            if (sName.rfind("Sampler_", 0) == 0)
                pairedSamplerTexNames.insert(sName.substr(8)); // strip "Sampler_"
        }

        // Sampled Images (GLSL-style CombinedImageSampler, rare for HLSL input)
        for (auto& img : resources.sampled_images)
        {
            ReflectedTexture reflected{};
            reflected.Name = compiler.get_name(img.id);
            reflected.Set = compiler.get_decoration(img.id, spv::DecorationDescriptorSet);
            reflected.Binding = compiler.get_decoration(img.id, spv::DecorationBinding);
            reflected.IsCombined = true;
            reflected.StageFlags = stageFlag;
            result.SampledImages.push_back(std::move(reflected));
        }

        // Separate images (HLSL Texture2D → separate_images in SPIR-V)
        for (auto& img : resources.separate_images)
        {
            ReflectedTexture reflected{};
            reflected.Name = compiler.get_name(img.id);
            reflected.Set = compiler.get_decoration(img.id, spv::DecorationDescriptorSet);
            reflected.Binding = compiler.get_decoration(img.id, spv::DecorationBinding);
            reflected.IsCombined = (pairedSamplerTexNames.count(reflected.Name) > 0);
            reflected.StageFlags = stageFlag;
            result.SampledImages.push_back(std::move(reflected));
        }

        return result;
    }
}
