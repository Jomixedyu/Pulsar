#pragma once
#include "psc/ShaderCompiler.h"

#include <vector>
#include <fstream>
#include <filesystem>
#include <chrono>
#include "IOHelper.hpp"

#include "../glslang/glslang/Include/Common.h"
#include "../glslang/glslang/Public/ShaderLang.h"
#include "../glslang/SPIRV/GlslangToSpv.h"
#include "../glslang/StandAlone/DirStackFileIncluder.h"
#include "../spirv-cross/spirv_glsl.hpp"

namespace psc
{
    static TBuiltInResource GetDefaultTBuiltInResource()
    {
        TBuiltInResource res{};
        res.maxLights = 32;
        res.maxClipPlanes = 6;
        res.maxTextureUnits = 32;
        res.maxTextureCoords = 32;
        res.maxVertexAttribs = 64;
        res.maxVertexUniformComponents = 4096;
        res.maxVaryingFloats = 64;
        res.maxVertexTextureImageUnits = 32;
        res.maxCombinedTextureImageUnits = 80;
        res.maxTextureImageUnits = 32;
        res.maxFragmentUniformComponents = 4096;
        res.maxDrawBuffers = 32;
        res.maxVertexUniformVectors = 128;
        res.maxVaryingVectors = 8;
        res.maxFragmentUniformVectors = 16;
        res.maxVertexOutputVectors = 16;
        res.maxFragmentInputVectors = 15;
        res.minProgramTexelOffset = -8;
        res.maxProgramTexelOffset = 7;
        res.maxClipDistances = 8;
        res.maxComputeWorkGroupCountX = 65535;
        res.maxComputeWorkGroupCountY = 65535;
        res.maxComputeWorkGroupCountZ = 65535;
        res.maxComputeWorkGroupSizeX = 1024;
        res.maxComputeWorkGroupSizeY = 1024;
        res.maxComputeWorkGroupSizeZ = 64;
        res.maxComputeUniformComponents = 1024;
        res.maxComputeTextureImageUnits = 16;
        res.maxComputeImageUniforms = 8;
        res.maxComputeAtomicCounters = 8;
        res.maxComputeAtomicCounterBuffers = 1;
        res.maxVaryingComponents = 60;
        res.maxVertexOutputComponents = 64;
        res.maxGeometryInputComponents = 64;
        res.maxGeometryOutputComponents = 128;
        res.maxFragmentInputComponents = 128;
        res.maxImageUnits = 8;
        res.maxCombinedImageUnitsAndFragmentOutputs = 8;
        res.maxCombinedShaderOutputResources = 8;
        res.maxImageSamples = 0;
        res.maxVertexImageUniforms = 0;
        res.maxTessControlImageUniforms = 0;
        res.maxTessEvaluationImageUniforms = 0;
        res.maxGeometryImageUniforms = 0;
        res.maxFragmentImageUniforms = 8;
        res.maxCombinedImageUniforms = 8;
        res.maxGeometryTextureImageUnits = 16;
        res.maxGeometryOutputVertices = 256;
        res.maxGeometryTotalOutputComponents = 1024;
        res.maxGeometryUniformComponents = 1024;
        res.maxGeometryVaryingComponents = 64;
        res.maxTessControlInputComponents = 128;
        res.maxTessControlOutputComponents = 128;
        res.maxTessControlTextureImageUnits = 16;
        res.maxTessControlUniformComponents = 1024;
        res.maxTessControlTotalOutputComponents = 4096;
        res.maxTessEvaluationInputComponents = 128;
        res.maxTessEvaluationOutputComponents = 128;
        res.maxTessEvaluationTextureImageUnits = 16;
        res.maxTessEvaluationUniformComponents = 1024;
        res.maxTessPatchComponents = 120;
        res.maxPatchVertices = 32;
        res.maxTessGenLevel = 64;
        res.maxViewports = 16;
        res.maxVertexAtomicCounters = 0;
        res.maxTessControlAtomicCounters = 0;
        res.maxTessEvaluationAtomicCounters = 0;
        res.maxGeometryAtomicCounters = 0;
        res.maxFragmentAtomicCounters = 8;
        res.maxCombinedAtomicCounters = 8;
        res.maxAtomicCounterBindings = 1;
        res.maxVertexAtomicCounterBuffers = 0;
        res.maxTessControlAtomicCounterBuffers = 0;
        res.maxTessEvaluationAtomicCounterBuffers = 0;
        res.maxGeometryAtomicCounterBuffers = 0;
        res.maxFragmentAtomicCounterBuffers = 1;
        res.maxCombinedAtomicCounterBuffers = 1;
        res.maxAtomicCounterBufferSize = 16384;
        res.maxTransformFeedbackBuffers = 4;
        res.maxTransformFeedbackInterleavedComponents = 64;
        res.maxCullDistances = 8;
        res.maxCombinedClipAndCullDistances = 8;
        res.maxSamples = 4;

        res.limits.nonInductiveForLoops = 1;
        res.limits.whileLoops = 1;
        res.limits.doWhileLoops = 1;
        res.limits.generalUniformIndexing = 1;
        res.limits.generalAttributeMatrixVectorIndexing = 1;
        res.limits.generalVaryingIndexing = 1;
        res.limits.generalSamplerIndexing = 1;
        res.limits.generalVariableIndexing = 1;
        res.limits.generalConstantMatrixVectorIndexing = 1;

        return res;
    }

    class GlslangCompilerImpl : public ShaderCompiler
    {
    public:
        EShLanguage GetShLanguage(FilePartialType type)
        {
            switch (type)
            {
            case FilePartialType::Vert: return EShLanguage::EShLangVertex;
            case FilePartialType::Pixel: return EShLanguage::EShLangFragment;
            case FilePartialType::Compute:return EShLanguage::EShLangCompute;
            case FilePartialType::Geometry:return EShLanguage::EShLangGeometry;
            case FilePartialType::TessControl:return EShLanguage::EShLangTessControl;
            case FilePartialType::TessEval:return EShLanguage::EShLangTessEvaluation;
            }
            return {};
        }
        glslang::EShClient GetShClient(ApiPlatformType type)
        {
            switch (type)
            {
            case ApiPlatformType::Vulkan: return glslang::EShClient::EShClientVulkan;
            case ApiPlatformType::Direct3D: break;
            case ApiPlatformType::Metal: break;
            case ApiPlatformType::OpenGL: return glslang::EShClient::EShClientOpenGL;
            }
            return {};
        }
        glslang::EShSource GetShSource(int sourceFileType)
        {
            switch (sourceFileType)
            {
            case SFT_glsl: return glslang::EShSourceGlsl;
            case SFT_hlsl: return glslang::EShSourceHlsl;
            default:
                break;
            }
            return {};
        }
        virtual std::vector<char> CompileStage(
            const char* code, 
            ApiPlatformType platform, 
            FilePartialType Stage, 
            const CompileInfo& compileInfo,
            const char* extraDebugPath)
        {
            auto langStage = GetShLanguage(Stage);
            glslang::TShader shader(langStage);
            shader.setStrings(&code, 1);

            glslang::EShClient client = GetShClient(platform);

            int ClientInputSemanticsVersion = 100;
            glslang::EShTargetClientVersion ClientVersion = glslang::EShTargetVulkan_1_2;


            glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_3;

            shader.setEnvInput(glslang::EShSourceHlsl, langStage, client, ClientInputSemanticsVersion);
            shader.setEnvClient(client, ClientVersion);
            shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

            TBuiltInResource resources{};
            resources = GetDefaultTBuiltInResource();

            EShMessages messages;
            messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

            const int DefaultVersion = 100;

            DirStackFileIncluder includer;

            for (auto& path : compileInfo.IncludePaths)
            {
                includer.pushExternalLocalDirectory(path.string());
            }

            std::string PreprocessedGLSL;

            if (!shader.preprocess(&resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, includer))
            {
                std::cout << shader.getInfoLog() << std::endl;
                std::cout << shader.getInfoDebugLog() << std::endl;
                throw std::runtime_error("Preprocessing Failed");
            }

            const char* PreprocessedCStr = PreprocessedGLSL.c_str();
            shader.setStrings(&PreprocessedCStr, 1);

            if (!shader.parse(&resources, 100, false, messages))
            {
                auto str = std::string("shader parsing failed: ") + shader.getInfoLog() + shader.getInfoDebugLog();
                if (extraDebugPath)
                {
                    str += "path: ";
                    str += extraDebugPath;
                }

                throw std::runtime_error(str);
            }

            glslang::TProgram Program;
            Program.addShader(&shader);

            if (!Program.link(messages))
            {
                std::cout << shader.getInfoLog() << std::endl;
                std::cout << shader.getInfoDebugLog() << std::endl;
                throw std::runtime_error("Linking Failed");
            }

            if (!Program.mapIO())
            {
                std::cout << shader.getInfoLog() << std::endl;
                std::cout << shader.getInfoDebugLog() << std::endl;
                throw std::runtime_error("Linking (Mapping IO) Failed");
            }

            std::vector<unsigned int> SpirV;
            spv::SpvBuildLogger logger;
            glslang::SpvOptions spvOptions;

            spvOptions.generateDebugInfo = compileInfo.Debug;

            glslang::GlslangToSpv(*Program.getIntermediate(langStage), SpirV, &logger, &spvOptions);

            std::cout << logger.getAllMessages() << std::endl;

            std::vector<char> spirData(SpirV.size() * sizeof(int));
            memcpy(spirData.data(), SpirV.data(), SpirV.size() * sizeof(int));

            if (platform == ApiPlatformType::OpenGL)
            {
                namespace spvc = spirv_cross;
                spvc::CompilerGLSL glsl{ SpirV };

                spvc::CompilerGLSL::Options options;
                options.version = 310;
                options.es = true;

                glsl.set_common_options(options);

                auto glslCode = glsl.compile();

                return std::vector<char>(glslCode.begin(), glslCode.end());
            }
            else if (platform == ApiPlatformType::OpenGLES)
            {

            }


            return spirData;
        }


        std::unordered_map<std::string, FilePartialType> _ShaderSrcExts
        {
            {"sh", FilePartialType::Sh },
            { "vs", FilePartialType::Vert },
            { "ps", FilePartialType::Pixel },
            { "cs", FilePartialType::Compute },
            { "gs", FilePartialType::Geometry },
            { "tcs", FilePartialType::TessControl },
            { "tes", FilePartialType::TessEval },
        };

        struct ShaderFile
        {
            std::string Path;
            FilePartialType PartialType;
            int SourceFileType;
        };

        std::vector<ShaderFile> GetFamilyFiles(std::filesystem::path shPath)
        {
            auto clean = shPath;
            clean.replace_extension().replace_extension();

            std::vector<ShaderFile> files;
            for (auto& sh : _ShaderSrcExts)
            {
                auto file = clean.string() + "." + sh.first;
                SourceFileType shType;

                if (std::filesystem::exists(file + ".hlsl"))
                {
                    shType = SFT_hlsl;
                    file = file + ".hlsl";
                }
                else if (std::filesystem::exists(file + ".glsl"))
                {
                    shType = SFT_glsl;
                    file = file + ".glsl";
                }
                else
                {
                    continue;
                }

                ShaderFile newShaderFile;
                newShaderFile.Path = file;
                newShaderFile.SourceFileType = shType;
                newShaderFile.PartialType = sh.second;
                files.push_back(newShaderFile);
            }

            ShaderFile shFile;
            shFile.Path = shPath.string();
            shFile.PartialType = FilePartialType::Sh;
            files.push_back(shFile);

            return files;
        }
        virtual std::vector<TargetShader> CompilePSH(
            std::filesystem::path shPath,
            CompileInfo compileInfo,
            const std::vector<ApiPlatformType>& targetPlatforms) override
        {
            auto begintime = std::chrono::high_resolution_clock::now();

            std::vector<TargetShader> targetShaders;

            compileInfo.IncludePaths.push_back(shPath.parent_path());

            // calc resource count
            const auto familyFiles = GetFamilyFiles(shPath);
            const auto resourceCount = (familyFiles.size() - 1) * targetPlatforms.size() + 1;

            std::cout << "compile shader: " << shPath << ". resource: " << resourceCount << std::endl;

            // write config text
            {
                auto& shCfg = targetShaders.emplace_back();
                shCfg.Name = "sh.cfg";
                shCfg.Partial = FilePartialType::Sh;
                shCfg.Platform = ApiPlatformType::Generic;
                shCfg.Data = IOHelper::ReadAllText(shPath);
            }

            glslang::InitializeProcess();
            //write platform
            for (int i = 0; i < targetPlatforms.size(); ++i)
            {
                auto& platform = targetPlatforms[i];

                for (auto& file : familyFiles)
                {
                    
                    if (file.SourceFileType & SFT_ShaderSrc)
                    {
                        auto& targetShader = targetShaders.emplace_back();
                        targetShader.Partial = file.PartialType;
                        targetShader.Platform = platform;
                        targetShader.Name = std::string(to_string(file.PartialType)) + "." + to_string(platform);

                        auto fileContent = IOHelper::ReadAllText(file.Path);
                        targetShader.Data = CompileStage(fileContent.c_str(), platform, file.PartialType, compileInfo, file.Path.c_str());
                    }
                    else
                    {
                        continue;
                    }

                }
            }

            glslang::FinalizeProcess();

            auto elapsed = std::chrono::high_resolution_clock::now() - begintime;
            std::cout << "elapsed time: " << std::chrono::duration<float, std::chrono::milliseconds::period>(elapsed).count() << "ms" << std::endl;

            return targetShaders;
        }

        virtual void CompileBinaryPSH(
            std::filesystem::path shPath,
            CompileInfo compileInfo,
            const std::vector<ApiPlatformType>& targetPlatforms,
            std::ostream& out)
        {
            compileInfo.IncludePaths.push_back(shPath.parent_path());

            auto targetShaders = CompilePSH(shPath, compileInfo, targetPlatforms);

            // calc resource count
            const auto resourceCount = targetShaders.size();
            const auto dataBeginOffset = sizeof(binf::BinaryFileHeader) + resourceCount * sizeof(binf::BinaryResourceInfo);

            int resourceIndex = 0;

            // write header
            {
                binf::BinaryFileHeader binHeader;
                memset(&binHeader, 0, sizeof(binHeader));

                memcpy(&binHeader.Magic, ShaderFileMagic, sizeof(binHeader.Magic));
                binHeader.ResourceTableOffset = sizeof(binf::BinaryFileHeader);
                binHeader.ResourceCount = resourceCount;
                binHeader.DataOffset = dataBeginOffset;

                memset(&binHeader.PlaceA, 255, sizeof(binHeader.PlaceA));

                out.write((char*)&binHeader, sizeof(binHeader));
            }

            std::vector<binf::BinaryResourceInfo> resTable;
            auto dataChunkOffset = 0;
            //write res table
            for (auto& target : targetShaders)
            {
                binf::BinaryResourceInfo& info = resTable.emplace_back();
                memset(&info, 0, sizeof(info));

                strcpy(info.Name, target.Name.c_str());
                info.Type = (uint32_t)target.Partial;
                info.Platform = (uint32_t)target.Platform;
                if (auto arr = std::get_if<std::vector<char>>(&target.Data))
                {
                    info.Length = arr->size();
                }
                else if (auto str = std::get_if<std::string>(&target.Data))
                {
                    info.Length = str->length() + 1;
                }

                dataChunkOffset += info.Length;
                out.write((char*)&info, sizeof(info));
            }

            //write data
            for (auto& target : targetShaders)
            {
                const char* data;
                size_t len;

                if (auto arr = std::get_if<std::vector<char>>(&target.Data))
                {
                    data = arr->data();
                    len = arr->size() + 1;
                }
                else if (auto str = std::get_if<std::string>(&target.Data))
                {
                    data = str->c_str();
                    len = str->length() + 1;
                }

                out.write(data, len);
            }

        }

    };
}