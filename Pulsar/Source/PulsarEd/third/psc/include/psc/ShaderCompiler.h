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

    class PSC_API ShaderCompiler
    {
    public:
        virtual std::vector<char> CompileStage(
            const CompileInfo& compileInfo,
            const char* extraDebugPath = nullptr) = 0;
    };

    extern PSC_API std::shared_ptr<ShaderCompiler> CreateShaderCompiler(ApiPlatformType platform);
}
