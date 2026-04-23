#pragma once
#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXApplication.h>
#include <Pulsar/AssetObject.h>
#include <unordered_map>
#include <unordered_set>

namespace pulsared
{
    class ThumbnailCache final
    {
    public:
        static constexpr int ThumbnailSize = 128;

        explicit ThumbnailCache(gfx::GFXApplication* app);
        ~ThumbnailCache();

        gfx::GFXDescriptorSet_wp GetThumbnail(const jxcorlib::guid_t& assetGuid);
        void GenerateThumbnail(const jxcorlib::guid_t& assetGuid, pulsar::AssetObject* asset);
        bool IsFailed(const jxcorlib::guid_t& assetGuid) const;
        void ClearCache();

    private:
        gfx::GFXDescriptorSet_wp LoadFromDisk(const jxcorlib::guid_t& assetGuid);
        void SaveToDisk(const jxcorlib::guid_t& assetGuid, const uint8_t* rgba, int width, int height);
        gfx::GFXDescriptorSet_wp CreateThumbnailTexture(const uint8_t* rgba, int width, int height);

        static std::vector<uint8_t> ResizeImageRGBA(const uint8_t* src, int srcW, int srcH, int dstW, int dstH);

        gfx::GFXApplication* m_app;
        gfx::GFXDescriptorSetLayout_sp m_descriptorLayout;
        std::unordered_map<jxcorlib::guid_t, gfx::GFXDescriptorSet_sp> m_descSets;
        std::unordered_map<jxcorlib::guid_t, gfx::GFXTexture_sp> m_textures;
        std::unordered_set<jxcorlib::guid_t> m_failedGuids;
    };
} // namespace pulsared
