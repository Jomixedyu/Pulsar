#include "RenderProxyRegistry.h"
#include "RenderProxy.h"

namespace pulsar::rendering
{
    void RenderProxyRegistry::RegisterProxy(RenderProxy* proxy)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_proxies.insert(proxy);
    }

    void RenderProxyRegistry::UnregisterProxy(RenderProxy* proxy)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_proxies.erase(proxy);
    }

    void RenderProxyRegistry::GetProxiesByInterface(const std::string& interfaceName,
                                                     std::vector<RenderProxy*>& out) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto* proxy : m_proxies)
        {
            if (proxy->GetInterface() == interfaceName)
            {
                out.push_back(proxy);
            }
        }
    }
}
