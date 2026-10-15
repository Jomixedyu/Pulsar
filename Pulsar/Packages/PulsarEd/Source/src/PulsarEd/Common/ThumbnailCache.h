#pragma once
#include "Common/PersistentTexturePool.h"
#include <Pulsar/AssetObject.h>
#include <unordered_set>
#include <vector>

namespace pulsared
{
    // Thumbnail storage lives in a PersistentTexturePool (texture assets +
    // published imgui image handles); this class only handles generating and
    // disk-caching the thumbnail pixels. Main thread only.
    class ThumbnailCache final
    {
    public:
        static constexpr int ThumbnailSize = 128;

        ThumbnailCache() = default;
        ~ThumbnailCache() = default;

        // 0 = no thumbnail ready (caller may queue GenerateThumbnail).
        uint64_t GetThumbnail(const jxcorlib::guid_t& assetGuid);
        void GenerateThumbnail(const jxcorlib::guid_t& assetGuid, pulsar::AssetObject* asset);
        bool IsFailed(const jxcorlib::guid_t& assetGuid) const;
        void ClearCache();

    private:
        void LoadFromDisk(const jxcorlib::guid_t& assetGuid);
        void SaveToDisk(const jxcorlib::guid_t& assetGuid, const uint8_t* rgba, int width, int height);
        void RegisterThumbnail(const jxcorlib::guid_t& assetGuid, const uint8_t* rgba, int width, int height);

        static std::vector<uint8_t> ResizeImageRGBA(const uint8_t* src, int srcW, int srcH, int dstW, int dstH);

        PersistentTexturePool m_pool;
        std::unordered_set<jxcorlib::guid_t> m_failedGuids;
    };
} // namespace pulsared