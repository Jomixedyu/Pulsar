#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace psc
{
    struct ReflectedCBufferMember
    {
        std::string Name;
        uint32_t Offset;
        uint32_t Size;
    };

    struct ReflectedUniformBuffer
    {
        std::string Name;
        uint32_t Set;
        uint32_t Binding;
        uint32_t Size;
        std::vector<ReflectedCBufferMember> Members;
    };

    struct ReflectedTexture
    {
        std::string Name;
        uint32_t Set;
        uint32_t Binding;
    };

    struct ReflectedShaderResources
    {
        std::vector<ReflectedUniformBuffer> UniformBuffers;
        std::vector<ReflectedTexture> SampledImages;
    };

    // 从 SPIR-V 二进制中提取反射信息
    ReflectedShaderResources ReflectSpirvResources(const std::vector<char>& spirvData);
}
