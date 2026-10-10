#include "Common/PersistentTexturePool.h"

#include <Pulsar/Rendering/TextureProxy.h>
#include <gfx/GFXTexture.h>

namespace pulsared
{
    PersistentTexturePool::~PersistentTexturePool()
    {
        ClearCache();
    }

    void PersistentTexturePool::Register(const index_string& name, const pulsar::RCPtr<pulsar::Texture2D>& texture)
    {
        if (!texture)
            return;

        if (auto it = m_textures.find(name); it != m_textures.end() && it->second != texture)
            Unregister(name);

        texture->EnsureGPUResource();

        m_textures.insert_or_assign(name, texture);
    }

    void PersistentTexturePool::Unregister(const index_string& name)
    {
        auto it = m_textures.find(name);
        if (it == m_textures.end())
            return;
        it->second->DestroyGPUResource();
        m_textures.erase(it);
    }

    bool PersistentTexturePool::Contains(const index_string& name) const
    {
        return m_textures.find(name) != m_textures.end();
    }

    uint64_t PersistentTexturePool::GetTextureId(const index_string& name) const
    {
        auto it = m_textures.find(name);
        if (it == m_textures.end() || !it->second)
            return 0;
        auto proxy = it->second->GetTextureProxy();
        return proxy ? proxy->GetNativeViewHandle() : 0;
    }

    void PersistentTexturePool::ClearCache()
    {
        for (auto& [name, texture] : m_textures)
        {
            texture->DestroyGPUResource();
        }
        m_textures.clear();
    }
} // namespace pulsared