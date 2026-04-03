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

            auto result = ExecuteCompile(task);
            // push callback to main thread queue
            {
                std::lock_guard lock(m_callbackMutex);
                m_completedCallbacks.push_back({task.m_callback, std::move(result)});
            }
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

        // Convert psc::ShaderStageFlags to gfx::GFXGpuProgramStageFlags.
        // The two enums share the same bit values by design, so a static_cast is safe.
        auto toGfxStage = [](psc::ShaderStageFlags f) {
            return static_cast<gfx::GFXGpuProgramStageFlags>(static_cast<uint32_t>(f));
        };

        for (const auto& ub : reflected.UniformBuffers)
        {
            if (ub.Set != kMaterialDescSet)
                continue;

            layout.m_totalCBufferSize = std::max(layout.m_totalCBufferSize, ub.Size);
            layout.m_cbufferBindingPoint = ub.Binding;

            // Merge cbuffer stage flags across all SPIR-V stages
            layout.m_cbufferStageFlags = static_cast<gfx::GFXGpuProgramStageFlags>(
                static_cast<uint32_t>(layout.m_cbufferStageFlags) |
                static_cast<uint32_t>(toGfxStage(ub.StageFlags)));

            for (const auto& member : ub.Members)
            {
                bool found = false;
                for (auto& existing : layout.m_constantEntries)
                {
                    if (existing.m_name == member.Name)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    pulsar::CBufferEntry entry{};
                    entry.m_name = member.Name;
                    entry.m_offset = member.Offset;
                    entry.m_size = member.Size;
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

            auto gfxStage = toGfxStage(img.StageFlags);

            bool found = false;
            for (auto& existing : layout.m_textureEntries)
            {
                if (existing.m_name == img.Name)
                {
                    // Merge stage flags
                    existing.m_stageFlags = static_cast<gfx::GFXGpuProgramStageFlags>(
                        static_cast<uint32_t>(existing.m_stageFlags) |
                        static_cast<uint32_t>(gfxStage));
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                pulsar::TextureEntry entry{};
                entry.m_name = img.Name;
                entry.m_bindingPoint = img.Binding;
                entry.m_isCombinedImageSampler = img.IsCombined;
                entry.m_stageFlags = gfxStage;
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
        // Disabled: always recompile (shader changes frequently)
        return false;

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

    pulsar::ShaderCompileResult EditorShaderCompileService::ExecuteCompile(const pulsar::ShaderCompileTask& task)
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
            // 拼一个可读名字，方便断点调试时识别是哪个 shader variant
            {
                std::string name = hlslAssetPath;
                if (!task.m_variantKey.m_passName.empty())
                    name += " [" + task.m_variantKey.m_passName;
                if (!task.m_variantKey.m_interface.empty())
                    name += "|" + task.m_variantKey.m_interface;
                if (!task.m_variantKey.m_passName.empty())
                    name += "]";
                program->m_shaderName = std::move(name);
            }

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

            // 使用 CompilePass 将 VS + PS 放入同一 TProgram 编译，
            // 通过 HlslBindingResolver 保证跨 stage binding 编号一致。
            psc::PassCompileInfo passInfo{};
            passInfo.code = hlslSource.c_str();
            passInfo.platform = pscApi;
            passInfo.vsEntry = task.m_entries.m_vertex;
            passInfo.psEntry = task.m_entries.m_fragment;
            passInfo.PreDefines = defines;
            passInfo.IncludePaths = { includeDir };
            passInfo.Debug = false;

            auto passResult = pscCompiler->CompilePass(passInfo, hlslAssetPath.c_str());

            // VS SPIR-V — 创建 GpuProgram，并反射活跃资源加入布局
            if (!task.m_entries.m_vertex.empty() && !passResult.vsSpirv.empty())
            {
                auto& spirv = passResult.vsSpirv;
                WriteToDiskCache(cacheHash, "vs", spirv);

                auto gpuProgram = gfxApp->CreateGpuProgram(
                    gfx::GFXGpuProgramStageFlags::Vertex,
                    reinterpret_cast<const uint8_t*>(spirv.data()), spirv.size());
                gpuProgram->SetEntryName(task.m_entries.m_vertex);
                program->m_gpuPrograms.push_back(gpuProgram);

                // ReflectSpirvResources 只反射活跃变量，死绑定不会出现
                auto reflected = psc::ReflectSpirvResources(spirv);
                ExtractLayout(reflected, program->m_layout);
            }

            // PS SPIR-V — 创建 GpuProgram，并反射活跃资源合并入布局
            if (!task.m_entries.m_fragment.empty() && !passResult.psSpirv.empty())
            {
                auto& spirv = passResult.psSpirv;
                WriteToDiskCache(cacheHash, "ps", spirv);

                auto gpuProgram = gfxApp->CreateGpuProgram(
                    gfx::GFXGpuProgramStageFlags::Fragment,
                    reinterpret_cast<const uint8_t*>(spirv.data()), spirv.size());
                gpuProgram->SetEntryName(task.m_entries.m_fragment);
                program->m_gpuPrograms.push_back(gpuProgram);

                // ExtractLayout 内部有名字去重，VS+PS 共同活跃的资源只记录一次
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

        return result;
    }

    pulsar::ShaderCompileResult EditorShaderCompileService::CompileSync(const pulsar::ShaderCompileTask& task)
    {
        return ExecuteCompile(task);
    }
}
