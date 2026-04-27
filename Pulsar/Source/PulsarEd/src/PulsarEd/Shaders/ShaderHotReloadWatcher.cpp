#include "ShaderHotReloadWatcher.h"

#include <PulsarEd/AssetDatabase.h>
#include <Pulsar/Rendering/ShaderInstanceCache.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/Logger.h>
#include <CoreLib/File.h>
#include <CoreLib.Serialization/JsonSerializer.h>

namespace pulsared
{
    void ShaderHotReloadWatcher::Initialize()
    {
        m_fileTimes.clear();
        m_elapsedSinceLastScan = 0.0f;
        ScanForChanges();
    }

    void ShaderHotReloadWatcher::Terminate()
    {
        m_fileTimes.clear();
    }

    void ShaderHotReloadWatcher::Tick(float dt)
    {
        m_elapsedSinceLastScan += dt;
        if (m_elapsedSinceLastScan >= kScanInterval)
        {
            m_elapsedSinceLastScan = 0.0f;
            ScanForChanges();
        }
    }

    static bool IsShaderSourceFile(const std::filesystem::path& path)
    {
        auto ext = path.extension().u8string();
        return ext == u8".hlsl" || ext == u8".inc.hlsl";
    }

    static jxcorlib::guid_t ParseGuidFromPmeta(const std::filesystem::path& pmetaPath)
    {
        if (!std::filesystem::exists(pmetaPath))
            return {};

        try
        {
            auto json = FileUtil::ReadAllText(pmetaPath);
            auto meta = ser::JsonSerializer::Deserialize<AssetMetaData>(json);
            if (meta && meta->Guid)
                return meta->Guid;
        }
        catch (...)
        {
        }
        return {};
    }

    void ShaderHotReloadWatcher::ScanForChanges()
    {
        namespace fs = std::filesystem;

        std::vector<fs::path> changedHlsl;
        std::vector<fs::path> changedIncHlsl;
        std::vector<fs::path> discoveredFiles;

        for (const auto& package : AssetDatabase::GetPackageInfos())
        {
            fs::path shadersDir = fs::path(package.Path) / "Assets" / "Shaders";
            if (!fs::exists(shadersDir) || !fs::is_directory(shadersDir))
                continue;

            for (auto& entry : fs::recursive_directory_iterator(shadersDir))
            {
                if (!entry.is_regular_file())
                    continue;

                auto path = entry.path().lexically_normal();
                if (!IsShaderSourceFile(path))
                    continue;

                auto lastWrite = fs::last_write_time(path);
                auto it = m_fileTimes.find(path);
                if (it == m_fileTimes.end())
                {
                    m_fileTimes[path] = lastWrite;
                }
                else if (it->second != lastWrite)
                {
                    it->second = lastWrite;
                    auto ext = path.extension().u8string();
                    if (ext == u8".hlsl")
                        changedHlsl.push_back(path);
                    else
                        changedIncHlsl.push_back(path);
                }
            }
        }

        // 处理 include 文件变化：保守策略，重载所有 shader
        if (!changedIncHlsl.empty())
        {
            for (const auto& path : changedIncHlsl)
            {
                pulsar::Logger::Log("Shader include modified: " + path.string());
            }
            ReloadAllShaders();
        }

        // 处理 hlsl 文件变化：精确重载对应 shader
        for (const auto& path : changedHlsl)
        {
            pulsar::Logger::Log("Shader source modified: " + path.string());

            auto pmetaPath = path;
            pmetaPath.replace_extension(".pmeta");
            auto guid = ParseGuidFromPmeta(pmetaPath);
            if (guid)
            {
                ReloadShader(guid);
            }
            else
            {
                pulsar::Logger::Log("Failed to find .pmeta for shader: " + path.string(), pulsar::LogLevel::Warning);
            }
        }
    }

    void ShaderHotReloadWatcher::ReloadShader(const jxcorlib::guid_t& guid)
    {
        // 1. 从 ShaderInstanceCache 中清除该 shader 的所有 variant
        pulsar::ShaderInstanceCache::Instance().InvalidateShader(guid);

        // 2. 通知所有引用该 shader 的 Material 清理 binding
        auto shader = pulsar::RuntimeAssetManager::GetLoadedAssetByGuid<pulsar::Shader>(guid);
        if (shader)
        {
            pulsar::RuntimeObjectManager::NotifyDependencySource(
                shader->GetObjectHandle(), pulsar::DependencyObjectState::Modified);
            pulsar::Logger::Log("Shader hot-reload triggered: " + shader->GetName());
        }
    }

    void ShaderHotReloadWatcher::ReloadAllShaders()
    {
        pulsar::ShaderInstanceCache::Instance().Clear();

        // 通知所有已加载的 Shader 发送 Modified 消息，触发所有 Material 清理 binding
        pulsar::RuntimeObjectManager::ForEachObject([](const pulsar::RuntimeObjectInfo& info)
        {
            if (!info.Pointer)
                return;
            if (auto shader = ptr_cast<pulsar::Shader>(info.Pointer))
            {
                pulsar::RuntimeObjectManager::NotifyDependencySource(
                    shader->GetObjectHandle(), pulsar::DependencyObjectState::Modified);
            }
        });

        pulsar::Logger::Log("All shaders hot-reload triggered due to include file change.");
    }
}
