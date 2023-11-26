#pragma once
#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXApplication.h>
#include <CoreLib/index_string.hpp>
#include <unordered_map>

namespace pulsared
{
    class PersistentImagePool final
    {
    public:
        gfx::GFXDescriptorSet_wp GetDescriptorSet(index_string id);
        void Register(const index_string& id, const uint8_t* iconBuf, size_t length);
        void ClearCache();
        explicit PersistentImagePool(gfx::GFXApplication* app);
        ~PersistentImagePool();

    private:
        std::unordered_map<index_string, gfx::GFXDescriptorSet_sp> m_cacheDescSets;
        std::unordered_map<index_string, gfx::GFXTexture2D_sp> m_textures;
        gfx::GFXDescriptorSetLayout_sp m_descriptorLayout;
        gfx::GFXApplication* m_app;
    };
}