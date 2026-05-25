#pragma once
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace pulsar::rendering
{
    class RenderProxy;

    // Registry of all active RenderObjects (will become RenderProxy in Phase 3).
    // Lives on the Render Thread. Game Thread must use RenderThread::EnqueueCommand
    // to register / unregister.
    class RenderProxyRegistry
    {
    public:
        void RegisterProxy(RenderProxy* proxy);
        void UnregisterProxy(RenderProxy* proxy);

        const std::unordered_set<RenderProxy*>& GetProxies() const
        {
            return m_proxies;
        }

        // Filter by interface name (e.g. RENDERER_STATICMESH)
        void GetProxiesByInterface(const std::string& interfaceName,
                                    std::vector<RenderProxy*>& out) const;

    private:
        std::unordered_set<RenderProxy*> m_proxies;
        mutable std::mutex m_mutex; // Protects against concurrent Register/Unregister
    };
}
