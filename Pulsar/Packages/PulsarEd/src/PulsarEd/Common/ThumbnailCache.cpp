#include "Common/ThumbnailCache.h"

#include <algorithm>
#include <cmath>

#include <Pulsar/Assets/Texture2D.h>
#include <Pulsar/AssetManager.h>
#include <Pulsar/Application.h>
#include <Pulsar/Util/TextureSaveUtil.h>
#include <PulsarEd/AssetDatabase.h>
#include <PulsarEd/Workspace.h>
#include <gfx/GFXImage.h>
#include <CoreLib/File.h>
#include <CoreLib/Guid.h>
#include <filesystem>

namespace pulsared
{
    ThumbnailCache::ThumbnailCache(gfx::GFXApplication* app)
        : m_app(app)
    {
        gfx::GFXDescriptorSetLayoutDesc info{
            gfx::GFXDescriptorSetLayoutDesc(gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXGpuProgramStageFlags::Fragment, 0)};
        m_descriptorLayout = m_app->CreateDescriptorSetLayout(&info, 1);
    }

    ThumbnailCache::~ThumbnailCache() = default;

    gfx::GFXDescriptorSet_wp ThumbnailCache::GetThumbnail(const jxcorlib::guid_t& assetGuid)
    {
        if (!assetGuid)
            return {};

        auto it = m_descSets.find(assetGuid);
        if (it != m_descSets.end())
            return it->second;

        auto diskResult = LoadFromDisk(assetGuid);
        if (!diskResult.expired())
            return diskResult;

        return {};
    }

    void ThumbnailCache::GenerateThumbnail(const jxcorlib::guid_t& assetGuid, pulsar::AssetObject* asset)
    {
        if (!assetGuid || !asset)
            return;

        if (m_failedGuids.count(assetGuid))
            return;

        if (m_descSets.count(assetGuid))
            return;

        auto* texture2d = dynamic_cast<pulsar::Texture2D*>(asset);
        if (!texture2d)
        {
            m_failedGuids.insert(assetGuid);
            return;
        }

        const int srcW = texture2d->GetWidth();
        const int srcH = texture2d->GetHeight();
        if (srcW <= 0 || srcH <= 0)
        {
            m_failedGuids.insert(assetGuid);
            return;
        }

        std::vector<uint8_t> decodedRGBA;
        int decodedW = srcW;
        int decodedH = srcH;

        if (texture2d->IsOriginMemoryLoaded() && texture2d->GetOriginCompressedBinarySize() > 0)
        {
            if (texture2d->IsOriginMemoryCompressed())
            {
                decodedRGBA = gfx::LoadImageFromMemory(
                    texture2d->GetOriginMemoryData(),
                    texture2d->GetOriginCompressedBinarySize(),
                    nullptr, nullptr, nullptr, 4);
            }
            else
            {
                decodedRGBA.assign(
                    texture2d->GetOriginMemoryData(),
                    texture2d->GetOriginMemoryData() + texture2d->GetOriginCompressedBinarySize());
            }
        }

        if (decodedRGBA.empty())
        {
            m_failedGuids.insert(assetGuid);
            return;
        }

        size_t expectedSize = static_cast<size_t>(decodedW) * decodedH * 4;
        size_t expectedHDRSize3 = static_cast<size_t>(decodedW) * decodedH * 3 * sizeof(float);
        size_t expectedHDRSize4 = static_cast<size_t>(decodedW) * decodedH * 4 * sizeof(float);

        if (decodedRGBA.size() == expectedHDRSize3 || decodedRGBA.size() == expectedHDRSize4)
        {
            // HDR float data -> LDR RGBA8 with simple tone mapping
            int srcChannels = (decodedRGBA.size() == expectedHDRSize3) ? 3 : 4;
            const float* srcFloat = reinterpret_cast<const float*>(decodedRGBA.data());
            std::vector<uint8_t> ldrData(decodedW * decodedH * 4);
            for (int y = 0; y < decodedH; ++y)
            {
                for (int x = 0; x < decodedW; ++x)
                {
                    int srcIdx = (y * decodedW + x) * srcChannels;
                    int dstIdx = (y * decodedW + x) * 4;
                    for (int c = 0; c < 3; ++c)
                    {
                        float v = srcFloat[srcIdx + c];
                        v = v / (1.0f + v); // simple Reinhard tone mapping
                        ldrData[dstIdx + c] = static_cast<uint8_t>(std::clamp(v * 255.0f, 0.0f, 255.0f));
                    }
                    if (srcChannels == 4)
                    {
                        float a = srcFloat[srcIdx + 3];
                        a = a / (1.0f + a);
                        ldrData[dstIdx + 3] = static_cast<uint8_t>(std::clamp(a * 255.0f, 0.0f, 255.0f));
                    }
                    else
                    {
                        ldrData[dstIdx + 3] = 255;
                    }
                }
            }
            decodedRGBA = std::move(ldrData);
        }
        else if (decodedRGBA.size() != expectedSize)
        {
            m_failedGuids.insert(assetGuid);
            return;
        }

        std::vector<uint8_t> thumbRGBA;
        int thumbW = ThumbnailSize;
        int thumbH = ThumbnailSize;

        if (decodedW == thumbW && decodedH == thumbH)
        {
            thumbRGBA = std::move(decodedRGBA);
        }
        else
        {
            float aspect = static_cast<float>(decodedW) / static_cast<float>(decodedH);
            if (aspect > 1.0f)
                thumbH = static_cast<int>(thumbW / aspect);
            else
                thumbW = static_cast<int>(thumbH * aspect);
            thumbW = std::max(thumbW, 1);
            thumbH = std::max(thumbH, 1);

            thumbRGBA = ResizeImageRGBA(decodedRGBA.data(), decodedW, decodedH, thumbW, thumbH);
        }

        SaveToDisk(assetGuid, thumbRGBA.data(), thumbW, thumbH);

        gfx::GFXSamplerConfig config{};
        config.Filter = gfx::GFXSamplerFilter::Linear;
        config.AddressMode = gfx::GFXSamplerAddressMode::ClampToEdge;

        auto tex2d = m_app->CreateTexture2DFromMemory(
            thumbRGBA.data(), thumbW * thumbH * 4,
            thumbW, thumbH,
            gfx::GFXTextureFormat::R8G8B8A8_UNorm,
            config);

        if (!tex2d)
        {
            m_failedGuids.insert(assetGuid);
            return;
        }

        auto descSet = m_app->GetDescriptorManager()->GetDescriptorSet(m_descriptorLayout);
        auto desc = descSet->FindByBinding(0);
        if (!desc)
            desc = descSet->AddDescriptor("p", 0);
        desc->SetTextureSampler2D(tex2d->Get2DView().get());
        descSet->Submit();

        m_textures.emplace(assetGuid, tex2d);
        m_descSets.emplace(assetGuid, descSet);
    }

    bool ThumbnailCache::IsFailed(const jxcorlib::guid_t& assetGuid) const
    {
        return m_failedGuids.count(assetGuid) != 0;
    }

    void ThumbnailCache::ClearCache()
    {
        m_descSets.clear();
        m_textures.clear();
        m_failedGuids.clear();
    }

    static constexpr const char* ThumbnailExt = ".thumb";

    gfx::GFXDescriptorSet_wp ThumbnailCache::LoadFromDisk(const jxcorlib::guid_t& assetGuid)
    {
        if (!Workspace::IsOpened())
            return {};

        std::filesystem::path thumbDir = pulsar::Application::inst()->GetTempDirectory() / "Thumbnails";

        // Try new raw format first
        std::filesystem::path thumbPath = thumbDir / (assetGuid.to_string() + ThumbnailExt);
        if (std::filesystem::exists(thumbPath))
        {
            auto bytes = jxcorlib::FileUtil::ReadAllBytes(thumbPath);
            if (bytes.size() >= 8)
            {
                int32_t width = 0, height = 0;
                std::memcpy(&width, bytes.data(), 4);
                std::memcpy(&height, bytes.data() + 4, 4);

                if (width > 0 && height > 0)
                {
                    size_t expectedPixelSize = static_cast<size_t>(width) * height * 4;
                    if (bytes.size() == 8 + expectedPixelSize)
                    {
                        gfx::GFXSamplerConfig config{};
                        config.Filter = gfx::GFXSamplerFilter::Linear;
                        config.AddressMode = gfx::GFXSamplerAddressMode::ClampToEdge;

                        auto tex2d = m_app->CreateTexture2DFromMemory(
                            reinterpret_cast<const uint8_t*>(bytes.data() + 8), expectedPixelSize,
                            width, height,
                            gfx::GFXTextureFormat::R8G8B8A8_UNorm,
                            config);

                        if (tex2d)
                        {
                            auto descSet = m_app->GetDescriptorManager()->GetDescriptorSet(m_descriptorLayout);
                            auto desc = descSet->FindByBinding(0);
                            if (!desc)
                                desc = descSet->AddDescriptor("p", 0);
                            desc->SetTextureSampler2D(tex2d->Get2DView().get());
                            descSet->Submit();

                            m_textures.emplace(assetGuid, tex2d);
                            m_descSets.emplace(assetGuid, descSet);
                            return descSet;
                        }
                    }
                }
            }
        }

        // Fallback to legacy PNG format
        std::filesystem::path pngPath = thumbDir / (assetGuid.to_string() + ".png");
        if (std::filesystem::exists(pngPath))
        {
            auto bytes = jxcorlib::FileUtil::ReadAllBytes(pngPath);
            if (!bytes.empty())
            {
                int32_t width = 0, height = 0, channel = 0;
                auto rgba = gfx::LoadImageFromMemory(bytes.data(), bytes.size(), &width, &height, &channel, 4);
                if (!rgba.empty() && width > 0 && height > 0)
                {
                    gfx::GFXSamplerConfig config{};
                    config.Filter = gfx::GFXSamplerFilter::Linear;
                    config.AddressMode = gfx::GFXSamplerAddressMode::ClampToEdge;

                    auto tex2d = m_app->CreateTexture2DFromMemory(
                        rgba.data(), width * height * 4,
                        width, height,
                        gfx::GFXTextureFormat::R8G8B8A8_UNorm,
                        config);

                    if (tex2d)
                    {
                        auto descSet = m_app->GetDescriptorManager()->GetDescriptorSet(m_descriptorLayout);
                        auto desc = descSet->FindByBinding(0);
                        if (!desc)
                            desc = descSet->AddDescriptor("p", 0);
                        desc->SetTextureSampler2D(tex2d->Get2DView().get());
                        descSet->Submit();

                        m_textures.emplace(assetGuid, tex2d);
                        m_descSets.emplace(assetGuid, descSet);
                        return descSet;
                    }
                }
            }
        }

        return {};
    }

    void ThumbnailCache::SaveToDisk(const jxcorlib::guid_t& assetGuid, const uint8_t* rgba, int width, int height)
    {
        if (!Workspace::IsOpened())
            return;

        std::filesystem::path thumbDir = pulsar::Application::inst()->GetTempDirectory() / "Thumbnails";
        std::filesystem::create_directories(thumbDir);

        std::filesystem::path thumbPath = thumbDir / (assetGuid.to_string() + ThumbnailExt);
        std::string pathStr = thumbPath.generic_string();

        size_t pixelSize = static_cast<size_t>(width) * height * 4;
        std::vector<char> buffer;
        buffer.resize(8 + pixelSize);
        std::memcpy(buffer.data(), &width, 4);
        std::memcpy(buffer.data() + 4, &height, 4);
        std::memcpy(buffer.data() + 8, rgba, pixelSize);

        jxcorlib::FileUtil::WriteAllBytes(
            jxcorlib::StrToU8Path(pathStr),
            buffer.data(),
            buffer.size());
    }

    std::vector<uint8_t> ThumbnailCache::ResizeImageRGBA(const uint8_t* src, int srcW, int srcH, int dstW, int dstH)
    {
        std::vector<uint8_t> dst(dstW * dstH * 4);

        const float xScale = static_cast<float>(srcW) / static_cast<float>(dstW);
        const float yScale = static_cast<float>(srcH) / static_cast<float>(dstH);

        for (int y = 0; y < dstH; ++y)
        {
            for (int x = 0; x < dstW; ++x)
            {
                float srcX = (x + 0.5f) * xScale - 0.5f;
                float srcY = (y + 0.5f) * yScale - 0.5f;

                int x0 = static_cast<int>(std::floor(srcX));
                int y0 = static_cast<int>(std::floor(srcY));
                int x1 = x0 + 1;
                int y1 = y0 + 1;

                float fx = srcX - x0;
                float fy = srcY - y0;

                x0 = std::clamp(x0, 0, srcW - 1);
                x1 = std::clamp(x1, 0, srcW - 1);
                y0 = std::clamp(y0, 0, srcH - 1);
                y1 = std::clamp(y1, 0, srcH - 1);

                for (int c = 0; c < 4; ++c)
                {
                    float v00 = src[(y0 * srcW + x0) * 4 + c];
                    float v10 = src[(y0 * srcW + x1) * 4 + c];
                    float v01 = src[(y1 * srcW + x0) * 4 + c];
                    float v11 = src[(y1 * srcW + x1) * 4 + c];

                    float v0 = v00 * (1.0f - fx) + v10 * fx;
                    float v1 = v01 * (1.0f - fx) + v11 * fx;
                    float v = v0 * (1.0f - fy) + v1 * fy;

                    dst[(y * dstW + x) * 4 + c] = static_cast<uint8_t>(std::clamp(v, 0.0f, 255.0f));
                }
            }
        }

        return dst;
    }
} // namespace pulsared
