#include "ShaderHotReloadWatcher.h"

#include <PulsarEd/AssetDatabase.h>
#include <Pulsar/Rendering/ShaderInstanceCache.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/Logger.h>
#include <CoreLib/File.h>
#include <CoreLib.Serialization/JsonSerializer.h>
#include <algorithm>
#include <cctype>

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

    static bool IsHlslFile(const std::filesystem::path& path)
    {
        auto filename = path.filename().string();
        std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
        // Exclude .inc.hlsl include files — they are not standalone shader assets
        if (filename.ends_with(".inc.hlsl"))
            return false;
        auto ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == ".hlsl";
    }

    static bool IsShaderSourceFile(const std::filesystem::path& path)
    {
        return IsHlslFile(path);
    }

    static bool IsPaFile(const std::filesystem::path& path)
    {
        auto ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == ".pa";
    }

    static SPtr<AssetMetaData> ParseMetaFromPmeta(const std::filesystem::path& pmetaPath)
    {
        if (!std::filesystem::exists(pmetaPath))
            return {};

        try
        {
            auto json = FileUtil::ReadAllText(pmetaPath);
            return ser::JsonSerializer::Deserialize<AssetMetaData>(json);
        }
        catch (...)
        {
        }
        return {};
    }

    static jxcorlib::guid_t ParseGuidFromPmeta(const std::filesystem::path& pmetaPath)
    {
        if (auto meta = ParseMetaFromPmeta(pmetaPath))
        {
            if (meta->Guid)
                return meta->Guid;
        }
        return {};
    }

    void ShaderHotReloadWatcher::ScanForChanges()
    {
        namespace fs = std::filesystem;

        std::vector<fs::path> changedHlsl;
        std::vector<fs::path> changedPa;

        for (const auto& package : AssetDatabase::GetPackageInfos())
        {
            fs::path assetsDir = fs::path(package.Path) / "Assets";
            if (!fs::exists(assetsDir) || !fs::is_directory(assetsDir))
                continue;

            try
            {
                for (auto& entry : fs::recursive_directory_iterator(assetsDir, fs::directory_options::skip_permission_denied))
                {
                    if (!entry.is_regular_file())
                        continue;

                    auto path = entry.path().lexically_normal();

                    std::filesystem::file_time_type lastWrite;
                    try
                    {
                        lastWrite = fs::last_write_time(path);
                    }
                    catch (...)
                    {
                        continue;
                    }

                    auto it = m_fileTimes.find(path);
                    if (it == m_fileTimes.end())
                    {
                        m_fileTimes[path] = lastWrite;
                    }
                    else if (it->second != lastWrite)
                    {
                        it->second = lastWrite;
                        auto filename = path.filename().string();
                        std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
                        if (filename.ends_with(".hlsl"))
                            changedHlsl.push_back(path);
                        else if (IsPaFile(path))
                            changedPa.push_back(path);
                    }
                }
            }
            catch (const std::exception& e)
            {
                pulsar::Logger::Log("Shader hot-reload scan error in " + assetsDir.string() + ": " + e.what(), pulsar::LogLevel::Error);
            }
        }

        // 处理 .pa 资产文件变化：重新加载 shader 配置数据
        for (const auto& path : changedPa)
        {
            pulsar::Logger::Log("Shader asset modified: " + path.string());
            ReloadShaderPa(path);
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

    void ShaderHotReloadWatcher::ReloadShaderPa(const std::filesystem::path& paPath)
    {
        auto pmetaPath = paPath;
        pmetaPath.replace_extension(".pmeta");

        auto meta = ParseMetaFromPmeta(pmetaPath);
        if (!meta || !meta->Guid)
        {
            pulsar::Logger::Log("Failed to parse .pmeta for: " + paPath.string(), pulsar::LogLevel::Warning);
            return;
        }

        if (meta->Type != "pulsar::Shader")
            return; // Not a shader asset, ignore

        auto shader = pulsar::RuntimeAssetManager::GetLoadedAssetByGuid<pulsar::Shader>(meta->Guid);
        if (!shader)
        {
            // Shader not currently loaded, no need to hot-reload
            return;
        }

        auto pbaPath = paPath;
        pbaPath.replace_extension(".pba");

        try
        {
            auto fileJson = FileUtil::ReadAllText(paPath);
            auto objser = ser::CreateVarient("json");
            objser->AssignParse(fileJson);

            auto serializer = pulsar::AssetSerializer{objser, pbaPath, false, true};
            shader->Serialize(&serializer);

            pulsar::Logger::Log("Shader asset data reloaded: " + paPath.string());
        }
        catch (const std::exception& e)
        {
            pulsar::Logger::Log("Failed to reload shader .pa: " + paPath.string() + ", error: " + e.what(), pulsar::LogLevel::Error);
            return;
        }

        // Invalidate cache and notify dependents
        ReloadShader(meta->Guid);
    }
}
