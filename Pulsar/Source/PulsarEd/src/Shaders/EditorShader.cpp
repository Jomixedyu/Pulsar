#include "CoreLib.Serialization/JsonSerializer.h"
#include "EditorAppInstance.h"

#include "PulsarEd/ExclusiveTask.h"
#include <PulsarEd/AssetDatabase.h>
#include <PulsarEd/Shaders/EditorShader.h>
#include <psc/ShaderCompiler.h>

namespace pulsared
{

    static psc::ApiPlatformType _GetApiType(gfx::GFXApi api)
    {
        switch (api)
        {
        case gfx::GFXApi::Unknown:
            break;
        case gfx::GFXApi::D3D12:
            return psc::ApiPlatformType::Direct3D;
            break;
        case gfx::GFXApi::Vulkan:
            return psc::ApiPlatformType::Vulkan;
            break;
        default:
            break;
        }

        throw NotImplementException();
    }
    static psc::FilePartialType _GetStage(gfx::GFXShaderStageFlags stage)
    {
        switch (stage)
        {
        case gfx::GFXShaderStageFlags::Vertex:
            return psc::FilePartialType::Vert;
            break;
        case gfx::GFXShaderStageFlags::Fragment:
            return psc::FilePartialType::Pixel;
            break;
        case gfx::GFXShaderStageFlags::VertexFragment:
            break;
        default:
            break;
        }
        throw NotImplementException();
    }
    static gfx::GFXShaderStageFlags _GetGFXStage(psc::FilePartialType type)
    {
        switch (type)
        {
        case psc::FilePartialType::Vert:
            return gfx::GFXShaderStageFlags::Vertex;
            break;
        case psc::FilePartialType::Pixel:
            return gfx::GFXShaderStageFlags::Fragment;
            break;
        case psc::FilePartialType::Compute:
            break;
        case psc::FilePartialType::Geometry:
            break;
        case psc::FilePartialType::TessControl:
            break;
        case psc::FilePartialType::TessEval:
            break;
        default:
            break;
        }

        throw NotImplementException();
    }



    std::vector<char> ShaderCompiler::CompileStageTargetPlatformByCode(
        const char* shaderCode, gfx::GFXApi api, gfx::GFXShaderStageFlags stage,
        const std::vector<std::filesystem::path>& includes, const std::vector<string>& defines)
    {
        auto pscApi = _GetApiType(api);
        auto pscCompiler = psc::CreateShaderCompiler(pscApi);
        auto pscStage = _GetStage(stage);

        psc::CompileInfo info{};
        info.IncludePaths = includes;
        info.PreDefines = defines;

        return pscCompiler->CompileStage(shaderCode, pscApi, pscStage, info);
    }

    std::vector<psc::TargetShader> ShaderCompiler::CompilePulsarShader(
        std::filesystem::path pshPath, 
        gfx::GFXApi api, 
        const std::vector<std::filesystem::path>& includes, 
        const std::vector<string>& defines)
    {
        auto pscApi = _GetApiType(api);
        auto pscCompiler = psc::CreateShaderCompiler(pscApi);

        psc::CompileInfo info{};
        info.IncludePaths = includes;
        info.PreDefines = defines;
        info.EntryName = "main";

        return pscCompiler->CompilePSH(pshPath, info, { pscApi });
    }

    void ShaderCompiler::CompileShader(
        Shader* shader,
        const array_list<gfx::GFXApi>& api,
        const std::vector<std::filesystem::path>& includes,
        const std::vector<string>& defines)
    {
        try
        {
            ShaderSourceData serDatas;
            auto passName = shader->GetPassName();

            if (passName->empty())
            {
                Logger::Log("passname is empty.", LogLevel::Error);
                return;
            }

            auto shaderPath = AssetDatabase::PackagePathToPhysicsPath(*passName);
            if (!exists(shaderPath))
            {
                auto assetPath = AssetDatabase::GetPathByAsset(shader);
                string errinfo = std::format("file not found: {}, asset: {}", passName->get_unboxing_value(), assetPath);
                throw std::ios_base::failure{ errinfo };
            }

            for (auto& apiItem : api)
            {
                auto& apiSerData = serDatas.ApiMaps[apiItem];

                auto passModules = CompilePulsarShader(shaderPath, apiItem, includes, defines);

                for (auto& smodule : passModules)
                {
                    if (smodule.Partial == psc::FilePartialType::Sh)
                    {
                        apiSerData.Config = std::get<string>(smodule.Data);
                    }
                    else
                    {
                        auto stage = _GetGFXStage(smodule.Partial);;
                        apiSerData.Sources[stage] = std::get<std::vector<char>>(smodule.Data);
                    }
                }
            }

            Logger::Log("compile shader success.");

            shader->ResetShaderSource(std::move(serDatas));

            AssetDatabase::MarkDirty(shader);
        }
        catch (const std::exception& e)
        {
            Logger::Log(e.what(), LogLevel::Error);
        }
    }
    void ShaderCompiler::CompileShader(Shader* shader)
    {
        //GetEdApp()->GetTaskQueue().AddTask();
        CompileShader(shader, Application::inst()->GetSupportedApis(), {}, {});
    }

} // namespace pulsared