#pragma once
#include <string>
#include <vector>
#include <variant>

namespace psc
{
    enum class FilePartialType : uint32_t
    {
        Sh, Vert, Pixel, Compute, Geometry, TessControl, TessEval,
    };
    inline const char* to_string(FilePartialType type)
    {
        switch (type)
        {
        case psc::FilePartialType::Sh: return "sh";
        case psc::FilePartialType::Vert: return "vs";
        case psc::FilePartialType::Pixel: return "ps";
        case psc::FilePartialType::Compute: return "cs";
        case psc::FilePartialType::Geometry: return "gs";
        case psc::FilePartialType::TessControl: return "tcs";
        case psc::FilePartialType::TessEval: return "tes";
        }
        return {};
    }
    enum SourceFileType : uint32_t
    {
        SFT_none,
        SFT_glsl = 1,
        SFT_hlsl = 1 << 1,
        SFT_yaml = 1 << 2,
        SFT_json = 1 << 3,
        SFT_xml = 1 << 4,
        SFT_toml = 1 << 5,
        SFT_ShaderSrc = SFT_glsl | SFT_hlsl,
        SFT_Config = SFT_yaml | SFT_json | SFT_xml | SFT_toml
    };

    enum class ApiPlatformType : uint32_t
    {
        None,
        Generic,
        Vulkan ,
        Direct3D,
        Metal ,
        OpenGL,
        OpenGLES,
    };
    inline const char* to_string(ApiPlatformType type)
    {
        switch (type)
        {
        case ApiPlatformType::None: return "None";
        case ApiPlatformType::Vulkan: return "Vulkan";
        case ApiPlatformType::Direct3D: return "Direct3D";
        case ApiPlatformType::Metal: return "Metal";
        case ApiPlatformType::OpenGL: return "OpenGL";
        }
        return nullptr;
    }


    constexpr const char* ShaderFileMagic = "plsrsh";

    struct TargetShader
    {
        std::string Name;
        ApiPlatformType Platform;
        FilePartialType Partial;
        std::variant<std::vector<char>, std::string> Data;
    };

}