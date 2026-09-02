#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
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
        const char* code;
        ApiPlatformType platform;
        FilePartialType Stage;
        bool Debug = false;
        IncludePaths IncludePaths;
        std::string EntryName;
        std::vector<std::string> PreDefines;
    };

    // Multi-stage pass compile: VS + PS compiled together in one TProgram
    // so binding numbers are consistent across stages.
    struct PassCompileInfo
    {
        const char* code = nullptr;
        ApiPlatformType platform = ApiPlatformType::None;
        bool Debug = false;
        IncludePaths IncludePaths;
        std::vector<std::string> PreDefines;
        std::string vsEntry;   // vertex shader entry point (empty = skip)
        std::string psEntry;   // pixel/fragment shader entry point (empty = skip)
    };

    struct PassCompileResult
    {
        std::vector<char> vsSpirv;  // empty if vsEntry was empty
        std::vector<char> psSpirv;  // empty if psEntry was empty
    };

    class PSC_API ShaderCompiler
    {
    public:
        virtual std::vector<char> CompileStage(
            const CompileInfo& compileInfo,
            const char* extraDebugPath = nullptr) = 0;

        // Compile all stages in one TProgram with unified binding assignment.
        virtual PassCompileResult CompilePass(
            const PassCompileInfo& passInfo,
            const char* extraDebugPath = nullptr) = 0;
    };

    extern PSC_API std::shared_ptr<ShaderCompiler> CreateShaderCompiler(ApiPlatformType platform);
}
