#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include "BinaryFileHeader.h"
#include "Classes.hpp"

#ifdef _WIN32
    #ifdef API_EXPORTS
        #define PSC_API __declspec(dllexport)
    #else
        #define PSC_API __declspec(dllimport)
    #endif
#else
#define PSC_API
#endif

namespace psc
{
    using IncludePaths = std::vector<std::filesystem::path>;

    struct CompileInfo
    {
        bool Debug = false;
        IncludePaths IncludePaths;
        std::vector<std::string> PreDefines;
    };

    class PSC_API ShaderCompiler
    {
    public:
        virtual std::vector<char> CompileStage(
            const char* code, 
            ApiPlatformType platform, 
            FilePartialType Stage, 
            const CompileInfo& compileInfo,
            const char* extraDebugPath = nullptr) = 0;

        virtual std::vector<TargetShader> CompilePSH(
            std::filesystem::path shPath,
            CompileInfo compileInfo,
            const std::vector<ApiPlatformType>& TargetPlatforms) = 0;

        virtual void CompileBinaryPSH(
            std::filesystem::path shPath,
            CompileInfo compileInfo,
            const std::vector<ApiPlatformType>& TargetPlatforms,
            std::ostream& out) = 0;


    };

    extern PSC_API std::shared_ptr<ShaderCompiler> CreateShaderCompiler(ApiPlatformType platform);
}
