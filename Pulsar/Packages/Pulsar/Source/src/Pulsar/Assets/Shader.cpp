#include "Assets/Shader.h"

#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Application.h>
#include <Pulsar/Rendering/RenderThread.h>
#include <Pulsar/Rendering/ShaderProxy.h>
#include <gfx/GFXResourceRegistry.h>
#include <CoreLib.Serialization/JsonSerializer.h>

#include "AssetManager.h"
#include "BuiltinAsset.h"
#include <Pulsar/Logger.h>

namespace pulsar
{
    using namespace std;

    RCPtr<Shader> Shader::StaticCreate(string_view name)
    {
        auto self = NewAssetObject<Shader>();
        self->SetName(name);

        return self;
    }

    Shader::Shader()
    {
        init_sptr_member(m_config);
    }

    void Shader::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);

        // Pure data: GPU-side re-sync after a hot reload is triggered by the
        // reloader (ShaderHotReloadWatcher), not from deserialization.
        if (!s->IsWrite) // read
        {
            m_config = ser::JsonSerializer::Deserialize<ShaderConfig>(s->Object->ToString());
        }
    }

    void Shader::OnDestroy()
    {
        DestroyGPUResource();
        base::OnDestroy();
    }

    void Shader::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        m_configDirty = true;
        EnsureGPUResource();
    }

    bool Shader::EnsureGPUResource()
    {
        if (!m_renderProxy)
        {
            // First call: create the proxy. The config may not be loaded yet
            // (created-but-not-deserialized asset) -- it arrives via later calls.
            m_renderProxy = std::make_shared<ShaderProxy>(GetAssetGuid(), m_config ? m_config->Clone() : nullptr);
            m_configDirty = false;
            m_isCreatedGPUResource = true;
            return true;
        }

        if (m_configDirty)
        {
            // Push a fresh config snapshot exactly once per change. The proxy may
            // be held and read by render-side consumers (MaterialProxy), so the
            // update goes through the render queue (UpdateConfig no-ops when
            // the content is identical).
            m_configDirty = false;
            if (auto* rt = Application::GetRenderThread())
            {
                auto proxy = m_renderProxy;
                auto config = m_config ? m_config->Clone() : nullptr;
                rt->EnqueueUpdate_AnyThread(
                    [proxy = std::move(proxy), config = std::move(config)](gfx::GFXResourceRegistry*) mutable
                    {
                        proxy->UpdateConfig(std::move(config));
                    });
            }
        }
        return true;
    }

    void Shader::DestroyGPUResource()
    {
        if (!m_renderProxy)
        {
            m_isCreatedGPUResource = false;
            return;
        }
        if (auto* rt = Application::GetRenderThread())
        {
            rt->EnqueueDestroy_AnyThread(
                [proxy = std::move(m_renderProxy)](gfx::GFXResourceRegistry*) mutable
                {
                    proxy.reset();
                });
        }
        else
        {
            m_renderProxy.reset();
        }
        m_isCreatedGPUResource = false;
    }

    void Shader::ReloadSceneProxy()
    {
        if (!m_renderProxy)
            return;
        if (auto* rt = Application::GetRenderThread())
        {
            rt->EnqueueUpdate_AnyThread(
                [proxy = m_renderProxy](gfx::GFXResourceRegistry*) mutable
                {
                    proxy->Reload();
                });
        }
    }
}
