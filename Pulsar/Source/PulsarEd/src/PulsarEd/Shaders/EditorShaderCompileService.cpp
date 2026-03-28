#include "EditorShaderCompileService.h"

#include <PulsarEd/AssetDatabase.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Application.h>

#include <gfx/GFXApi.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGpuProgram.h>
#include <psc/ShaderReflect.h>

#include <fstream>
#include <sstream>
#include <iomanip>

namespace pulsared
{
    static psc::ApiPlatformType GetPscApiType(gfx::GFXApi api)
    {
        switch (api)
        {
        case gfx::GFXApi::Vulkan:
            return psc::ApiPlatformType::Vulkan;
        case gfx::GFXApi::D3D12:
            return psc::ApiPlatformType::Direct3D;
        default:
            break;
        }
        return psc::ApiPlatformType::None;
    }

    static gfx::GFXGpuProgramStageFlags GetGFXStage(psc::FilePartialType type)
    {
        switch (type)
        {
        case psc::FilePartialType::Vert:
            return gfx::GFXGpuProgramStageFlags::Vertex;
        case psc::FilePartialType::Pixel:
            return gfx::GFXGpuProgramStageFlags::Fragment;
        default:
            break;
        }
        return gfx::GFXGpuProgramStageFlags::Vertex;
    }

    EditorShaderCompileService::EditorShaderCompileService()
    {
        // 默认缓存目录: 项目根目录下的 .shader_cache
        m_cacheDir = std::filesystem::current_path() / ".shader_cache";
        m_workerThread = std::thread(&EditorShaderCompileService::WorkerThread, this);
    }

    EditorShaderCompileService::~EditorShaderCompileService()
    {
        m_running = false;
        m_taskCV.notify_one();
        if (m_workerThread.joinable())
        {
            m_workerThread.join();
        }
    }

    void EditorShaderCompileService::RequestCompile(const pulsar::ShaderCompileTask& task)
    {
        {
            std::lock_guard lock(m_taskMutex);
            m_pendingTasks.push(task);
        }
        m_taskCV.notify_one();
    }

    void EditorShaderCompileService::FlushCallbacks()
    {
        std::vector<CompletedCallback> callbacks;
        {
            std::lock_guard lock(m_callbackMutex);
            callbacks.swap(m_completedCallbacks);
        }
        for (auto& cb : callbacks)
        {
            if (cb.m_callback)
            {
                cb.m_callback(cb.m_result);
            }
        }
    }

    void EditorShaderCompileService::WorkerThread()
    {
        while (m_running)
        {
            pulsar::ShaderCompileTask task;
            {
                std::unique_lock lock(m_taskMutex);
                m_taskCV.wait(lock, [this] { return !m_pendingTasks.empty() || !m_running; });

                if (!m_running && m_pendingTasks.empty())
                    break;

                task = std::move(m_pendingTasks.front());
                m_pendingTasks.pop();
            }

            ExecuteCompile(task);
        }
    }

    std::vector<std::string> EditorShaderCompileService::BuildDefines(const pulsar::ShaderCompileTask& task) const
    {
        std::vector<std::string> defines = task.m_defines;

        // Pass 宏: PASS_<NAME>
        if (!task.m_variantKey.m_passName.empty())
        {
            std::string passDefine = "PASS_";
            // 转大写
            for (char c : task.m_variantKey.m_passName)
            {
                passDefine += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }
            defines.push_back(passDefine);
        }

        // Interface 宏: 原值直接作为宏名
        if (!task.m_variantKey.m_interface.empty())
        {
            defines.push_back(task.m_variantKey.m_interface);
        }

        // Features 宏: 每个字符串直接作为宏
        for (const auto& feature : task.m_variantKey.m_features)
        {
            defines.push_back(feature);
        }

        return defines;
    }

    void EditorShaderCompileService::ExtractLayout(
        const psc::ReflectedShaderResources& reflected,
        pulsar::ShaderPropertyLayout& layout)
    {
        // 材质参数所在的 descriptor set（space0 = set 0）
        constexpr uint32_t kMaterialDescSet = 0;

        for (const auto& ub : reflected.UniformBuffers)
        {
            if (ub.Set != kMaterialDescSet)
                continue;

            layout.m_totalCBufferSize = std::max(layout.m_totalCBufferSize, ub.Size);

            for (const auto& member : ub.Members)
            {
                // 避免重复添加（VS/PS 可能有相同的 cbuffer）
                bool exists = false;
                for (const auto& existing : layout.m_constantEntries)
                {
                    if (existing.m_name == member.Name)
                    {
                        exists = true;
                        break;
                    }
                }
                if (!exists)
                {
                    pulsar::CBufferEntry entry{};
                    entry.m_name = member.Name;
                    entry.m_offset = member.Offset;
                    entry.m_size = member.Size;
                    // 根据 size 推断类型
                    if (member.Size == 4)
                        entry.m_type = pulsar::ShaderPropertyType::Float;
                    else if (member.Size == 16)
                        entry.m_type = pulsar::ShaderPropertyType::Float4;
                    else
                        entry.m_type = pulsar::ShaderPropertyType::Float;
                    layout.m_constantEntries.push_back(std::move(entry));
                }
            }
        }

        for (const auto& img : reflected.SampledImages)
        {
            if (img.Set != kMaterialDescSet)
                continue;

            // 避免重复
            bool exists = false;
            for (const auto& existing : layout.m_textureEntries)
            {
                if (existing.m_name == img.Name)
                {
                    exists = true;
                    break;
                }
            }
            if (!exists)
            {
                pulsar::TextureEntry entry{};
                entry.m_name = img.Name;
                entry.m_bindingPoint = img.Binding;
                layout.m_textureEntries.push_back(std::move(entry));
            }
        }
    }

    void EditorShaderCompileService::SetCacheDirectory(const std::filesystem::path& cacheDir)
    {
        m_cacheDir = cacheDir;
    }

    size_t EditorShaderCompileService::ComputeCacheHash(
        const std::string& hlslSource,
        const std::vector<std::string>& defines) const
    {
        // 将源码和所有 defines 拼接后计算 hash
        std::string combined = hlslSource;
        for (const auto& def : defines)
        {
            combined += '\0';
            combined += def;
        }
        return std::hash<std::string>{}(combined);
    }

    std::filesystem::path EditorShaderCompileService::GetCachePath(
        size_t hash,
        const std::string& stageSuffix) const
    {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(16) << hash << "." << stageSuffix << ".spv";
        return m_cacheDir / oss.str();
    }

    bool EditorShaderCompileService::TryLoadFromDiskCache(
        size_t hash,
        const std::string& stageSuffix,
        std::vector<char>& outSpirv) const
    {
        auto cachePath = GetCachePath(hash, stageSuffix);
        if (!std::filesystem::exists(cachePath))
            return false;

        std::ifstream file(cachePath, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            return false;

        auto size = file.tellg();
        file.seekg(0, std::ios::beg);
        outSpirv.resize(static_cast<size_t>(size));
        file.read(outSpirv.data(), size);
        return true;
    }

    void EditorShaderCompileService::WriteToDiskCache(
        size_t hash,
        const std::string& stageSuffix,
        const std::vector<char>& spirv) const
    {
        if (m_cacheDir.empty())
            return;

        std::filesystem::create_directories(m_cacheDir);

        auto cachePath = GetCachePath(hash, stageSuffix);
        std::ofstream file(cachePath, std::ios::binary);
        if (file.is_open())
        {
            file.write(spirv.data(), static_cast<std::streamsize>(spirv.size()));
        }
    }

    void EditorShaderCompileService::ExecuteCompile(const pulsar::ShaderCompileTask& task)
    {
        pulsar::ShaderCompileResult result{};

        // 从 guid 解析 HLSL 资产路径（提前到 try 外，catch 中可用于错误信息）
        auto assetPath = AssetDatabase::GetPathByGuid(task.m_variantKey.m_shaderGuid);
        auto dotPos = assetPath.rfind('.');
        auto hlslAssetPath = (dotPos != std::string::npos ? assetPath.substr(0, dotPos) : assetPath) + ".hlsl";

        try
        {
            auto pscApi = GetPscApiType(gfx::GFXApi::Vulkan); // TODO: 获取当前 API
            auto pscCompiler = psc::CreateShaderCompiler(pscApi);

            auto defines = BuildDefines(task);

            auto program = std::make_shared<pulsar::ShaderProgramResource>();
            program->m_key = task.m_variantKey;

            // 从 guid 解析 HLSL 物理路径
            auto hlslPhysicsPath = AssetDatabase::AssetPathToPhysicsPath(hlslAssetPath);

            auto includeDir = hlslPhysicsPath.parent_path();

            // 读取 HLSL 源码
            std::ifstream hlslFile(hlslPhysicsPath);
            auto hlslSource = std::string(
                (std::istreambuf_iterator<char>(hlslFile.rdbuf())),
                std::istreambuf_iterator<char>());

            // 计算缓存 hash（source + defines）
            auto cacheHash = ComputeCacheHash(hlslSource, defines);

            auto gfxApp = pulsar::Application::GetGfxApp();

            // 编译 Vertex Shader
            if (!task.m_entries.m_vertex.empty())
            {
                std::vector<char> spirv;

                if (TryLoadFromDiskCache(cacheHash, "vs", spirv))
                {
                    pulsar::Logger::Log("Shader VS cache hit: " + hlslAssetPath);
                }
                else
                {
                    psc::CompileInfo info{};
                    info.code = hlslSource.c_str();
                    info.platform = pscApi;
                    info.Stage = psc::FilePartialType::Vert;
                    info.EntryName = task.m_entries.m_vertex;
                    info.PreDefines = defines;
                    info.IncludePaths = { includeDir };

                    spirv = pscCompiler->CompileStage(info);
                    WriteToDiskCache(cacheHash, "vs", spirv);
                }

                // 从 SPIR-V 创建 GFXGpuProgram
                auto gpuProgram = gfxApp->CreateGpuProgram(
                    gfx::GFXGpuProgramStageFlags::Vertex,
                    reinterpret_cast<const uint8_t*>(spirv.data()), spirv.size());
                gpuProgram->SetEntryName(task.m_entries.m_vertex);
                program->m_gpuPrograms.push_back(gpuProgram);

                // 通过反射提取 ShaderPropertyLayout（从 VS 提取）
                auto reflected = psc::ReflectSpirvResources(spirv);
                ExtractLayout(reflected, program->m_layout);
            }

            // 编译 Fragment Shader
            if (!task.m_entries.m_fragment.empty())
            {
                std::vector<char> spirv;

                if (TryLoadFromDiskCache(cacheHash, "ps", spirv))
                {
                    pulsar::Logger::Log("Shader PS cache hit: " + hlslAssetPath);
                }
                else
                {
                    psc::CompileInfo info{};
                    info.code = hlslSource.c_str();
                    info.platform = pscApi;
                    info.Stage = psc::FilePartialType::Pixel;
                    info.EntryName = task.m_entries.m_fragment;
                    info.PreDefines = defines;
                    info.IncludePaths = { includeDir };

                    spirv = pscCompiler->CompileStage(info);
                    WriteToDiskCache(cacheHash, "ps", spirv);
                }

                // 从 SPIR-V 创建 GFXGpuProgram
                auto gpuProgram = gfxApp->CreateGpuProgram(
                    gfx::GFXGpuProgramStageFlags::Fragment,
                    reinterpret_cast<const uint8_t*>(spirv.data()), spirv.size());
                gpuProgram->SetEntryName(task.m_entries.m_fragment);
                program->m_gpuPrograms.push_back(gpuProgram);

                // 通过反射提取 ShaderPropertyLayout（合并 PS 的参数）
                auto reflected = psc::ReflectSpirvResources(spirv);
                ExtractLayout(reflected, program->m_layout);
            }

            result.m_success = true;
            result.m_program = program;

            pulsar::Logger::Log("Shader compiled successfully: " + hlslAssetPath);
        }
        catch (const std::exception& e)
        {
            result.m_success = false;
            result.m_errorMessage = e.what();

            pulsar::Logger::Log(
                std::string("Shader compile failed [") + hlslAssetPath + "]: " + e.what(),
                pulsar::LogLevel::Error);
        }

        // 将回调推入主线程队列
        {
            std::lock_guard lock(m_callbackMutex);
            m_completedCallbacks.push_back({task.m_callback, result});
        }
    }
}
