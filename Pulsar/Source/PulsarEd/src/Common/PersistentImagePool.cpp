#include "Common/PersistentImagePool.h"

#include "gfx/GFXImage.h"

namespace pulsared
{
    void PersistentImagePool::Register(const index_string& id, const uint8_t* iconBuf, size_t length)
    {
        gfx::GFXSamplerConfig config{};
        config.Filter = gfx::GFXSamplerFilter::Linear;

        int32_t width, height, channel;
        auto iconData = gfx::LoadImageFromMemory(iconBuf, length, &width, &height, &channel, 4, true);

        auto tex2d = m_app->CreateTexture2DFromMemory(iconData.data(), iconData.size(), width, height, gfx::GFXTextureFormat::R8G8B8A8_UNorm, config);
        m_textures.emplace(id, tex2d);
    }

    gfx::GFXDescriptorSet_wp PersistentImagePool::GetDescriptorSet(index_string id)
    {
        // read cache
        auto it = m_cacheDescSets.find(id);
        if (it != m_cacheDescSets.end())
        {
            return it->second;
        }

        // generate cache
        auto texIt = m_textures.find(id);
        if (texIt == m_textures.end())
        {
            return {};
        }
        auto tex = texIt->second;
        auto descSet = m_app->GetDescriptorManager()->GetDescriptorSet(m_descriptorLayout);

        auto desc = descSet->FindByBinding(0);
        if (!desc)
            desc = descSet->AddDescriptor("p", 0);

        desc->SetTextureSampler2D(tex.get());

        descSet->Submit();

        m_cacheDescSets.emplace(id, descSet);

        return descSet;
    }

    void PersistentImagePool::ClearCache()
    {
        m_cacheDescSets.clear();
    }
    PersistentImagePool::PersistentImagePool(gfx::GFXApplication* app)
        : m_app(app)
    {
        gfx::GFXDescriptorSetLayoutInfo info{
            gfx::GFXDescriptorSetLayoutInfo(gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXShaderStageFlags::Fragment, 0)};
        m_descriptorLayout = m_app->CreateDescriptorSetLayout(&info, 1);
    }

    PersistentImagePool::~PersistentImagePool()
    {
    }
} // namespace pulsared