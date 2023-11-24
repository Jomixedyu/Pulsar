#pragma once
#include <PulsarEd/Assembly.h>
#include <filesystem>
#include <psc/Classes.hpp>
#include <Pulsar/Assets/Shader.h>

namespace pulsared
{

    class ShaderCompiler
    {
    public:
        static std::vector<char> CompileStageTargetPlatformByCode(
            const char* shaderCode, gfx::GFXApi api, gfx::GFXShaderStageFlags stage,
            const std::vector<std::filesystem::path>& includes, const std::vector<string>& defines);

        static std::vector<psc::TargetShader> CompilePulsarShader(
            std::filesystem::path pshPath, gfx::GFXApi api,
            const std::vector<std::filesystem::path>& includes, const std::vector<string>& defines);

        static void CompileShader(
            Shader_ref shader,
            const array_list<gfx::GFXApi>& api,
            const std::vector<std::filesystem::path>& includes,
            const std::vector<string>& defines);
    };

}