#pragma once

#include <CoreLib/Guid.h>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace pulsared
{
    class ShaderHotReloadWatcher
    {
    public:
        void Initialize();
        void Terminate();
        void Tick(float dt);

    private:
        void ScanForChanges();
        void ReloadShader(const jxcorlib::guid_t& guid);
        void ReloadAllShaders();

        std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> m_fileTimes;
        float m_elapsedSinceLastScan = 0.0f;
        static constexpr float kScanInterval = 2.0f;
    };
}
