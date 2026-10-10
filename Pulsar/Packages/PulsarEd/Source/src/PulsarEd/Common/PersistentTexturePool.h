#pragma once
#include <Pulsar/Assets/Texture2D.h>
#include <CoreLib/index_string.hpp>
#include <cstdint>
#include <unordered_map>

namespace pulsared
{
    // Editor icon pool; a main-thread object. Stores texture ASSETS only --
    // the GPU objects live behind Texture2D's render-thread proxy, which also
    // publishes the native view handle. GetTextureId() returns 0 while the
    // texture is not created yet; draw a placeholder meanwhile.
    class PersistentTexturePool final
    {
    public:
        PersistentTexturePool() = default;
        ~PersistentTexturePool();

        // Registers a texture asset under a name and takes over its GPU
        // lifecycle: the GPU resource is created here and destroyed with the
        // pool. Main thread only.
        void Register(const index_string& name, const pulsar::RCPtr<pulsar::Texture2D>& texture);

        // Destroys the texture's GPU resource and drops the entry.
        void Unregister(const index_string& name);

        // True when the name is registered (its texture may still be creating).
        bool Contains(const index_string& name) const;

        // Main thread only. 0 = not ready yet.
        uint64_t GetTextureId(const index_string& name) const;

        void ClearCache();

    private:
        std::unordered_map<index_string, pulsar::RCPtr<pulsar::Texture2D>> m_textures;
    };
}