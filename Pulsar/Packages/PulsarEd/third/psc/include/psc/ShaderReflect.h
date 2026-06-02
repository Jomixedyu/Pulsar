#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace psc
{
    // Bitmask matching gfx::GFXGpuProgramStageFlags values intentionally,
    // so a static_cast is safe on the engine side.
    enum class ShaderStageFlags : uint32_t
    {
        None     = 0,
        Vertex   = 1,
        Fragment = 1 << 1,
        Compute  = 1 << 2,
        All      = 0xFFFFFFFF,
    };
    inline ShaderStageFlags operator|(ShaderStageFlags a, ShaderStageFlags b)
    {
        return static_cast<ShaderStageFlags>(
            static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    inline ShaderStageFlags& operator|=(ShaderStageFlags& a, ShaderStageFlags b)
    {
        return a = a | b;
    }

    enum class MemberBaseType : uint8_t
    {
        Unknown,
        Int,    // int32 / uint32 / bool — all stored as 4-byte integer
        Float,  // float32
    };

    struct ReflectedCBufferMember
    {
        std::string Name;
        uint32_t    Offset;
        uint32_t    Size;
        uint32_t    Columns = 1;   // vec/matrix column count (1 = scalar)
        MemberBaseType BaseType = MemberBaseType::Unknown;
    };

    struct ReflectedUniformBuffer
    {
        std::string Name;
        uint32_t Set;
        uint32_t Binding;
        uint32_t Size;
        ShaderStageFlags StageFlags = ShaderStageFlags::None;
        std::vector<ReflectedCBufferMember> Members;
    };

    struct ReflectedTexture
    {
        std::string Name;
        uint32_t Set;
        uint32_t Binding;
        bool IsCombined = false;
        ShaderStageFlags StageFlags = ShaderStageFlags::None;
    };

    struct ReflectedShaderResources
    {
        std::vector<ReflectedUniformBuffer> UniformBuffers;
        std::vector<ReflectedTexture> SampledImages;
    };

    // 从 SPIR-V 二进制中提取反射信息
    ReflectedShaderResources ReflectSpirvResources(const std::vector<char>& spirvData);
}
