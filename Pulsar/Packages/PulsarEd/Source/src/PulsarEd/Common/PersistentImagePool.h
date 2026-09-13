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
        std::unordered_map<index_string, gfx::GFXDescriptorSetPtr> m_cacheDescSets;
        std::unordered_map<index_string, gfx::GFXTexturePtr> m_textures;
        gfx::GFXDescriptorSetLayoutPtr m_descriptorLayout;
        gfx::GFXApplication* m_app;
    };
}
