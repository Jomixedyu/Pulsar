#include "Pulsar/Assets/CurveLinearColorAtlas.h"

#include "Application.h"
#include <Pulsar/Rendering/RenderThread.h>
#include <gfx/GFXResourceManager.h>

namespace pulsar
{

    CurveLinearColorAtlas::CurveLinearColorAtlas()
        : m_width(256), m_height(256)
    {
        init_sptr_member(m_colorCurveAssets);
    }
    void CurveLinearColorAtlas::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (s->IsWrite)
        {
            s->Object->Add("width", m_width);
            s->Object->Add("height", m_height);
            auto list = s->Object->New(ser::VarientType::Array);
            for (auto& element : *m_colorCurveAssets)
            {
                list->Push(element.GetGuid().to_string());
            }
            s->Object->Add("curves", list);
        }
        else
        {
            m_width = s->Object->At("width")->AsInt();
            m_height = s->Object->At("height")->AsInt();
            m_colorCurveAssets->clear();
            auto curves = s->Object->At("curves");
            for (int i = 0; i < curves->GetCount(); ++i)
            {
                WeakAssetPtr<CurveLinearColor> curve = curves->At(i)->AsString();
                m_colorCurveAssets->push_back(curve.Get());
            }
            RebuildObserver();
            Generate();
        }
    }

    void CurveLinearColorAtlas::Generate()
    {
        m_bitmap.resize(m_height * m_width);
        auto bitmapDataSize = m_bitmap.size() * sizeof(Color4b);
        memset(m_bitmap.data(), 0, bitmapDataSize);

        auto curveCount = m_colorCurveAssets->size();
        if (curveCount > m_height)
        {
            curveCount = m_height;
        }

        for (int row = 0; row < curveCount; ++row)
        {
            auto curveAsset = m_colorCurveAssets->at(row).GetPtr();

            if (curveAsset)
            {
                for (int column = 0; column < m_width; ++column)
                {
                    auto index = m_width * row + column;

                    auto t = (float)column / (float)m_width;
                    auto color = curveAsset->SampleColor(t);

                    m_bitmap[index] = Color4b(uint8_t(color.r*255), uint8_t(color.g*255), uint8_t(color.b*255), uint8_t(color.a*255));
                }
            }
        }

        // fill texture
        if (m_isCreatedGpuResource)
        {
            SamplerConfig cfg;
            cfg.Filter = GetSamplerFilter();
            cfg.AddressMode = GetSamplerAddressMode();

            auto* resMgr = Application::GetGfxApp()->GetResourceManager();
            auto* renderThread = Application::GetRenderThread();

            // 重新生成：先投递销毁旧句柄，再分配新句柄并投递创建。
            if (m_texHandle.IsValid())
            {
                renderThread->EnqueueUpdate_AnyThread(
                    [h = m_texHandle](gfx::GFXResourceManager* mgr) { mgr->Destroy(h); });
            }

            array_list<Color4b> bitmapCopy = m_bitmap;
            m_texHandle = resMgr->AllocHandle<gfx::TextureHandle>();
            renderThread->EnqueueUpdate_AnyThread(
                [h = m_texHandle, w = m_width, ht = m_height, cfg, bitmap = std::move(bitmapCopy)](gfx::GFXResourceManager* mgr)
                {
                    gfx::GFXTextureCreateDesc desc{};
                    desc.ImageData  = reinterpret_cast<const uint8_t*>(bitmap.data());
                    desc.DataLength = bitmap.size() * sizeof(Color4b);
                    desc.Width      = w;
                    desc.Height     = ht;
                    desc.Format     = gfx::GFXTextureFormat::R8G8B8A8_UNorm;
                    desc.SamplerCfg = cfg;
                    mgr->CreateTexture2D(h, desc);
                });
        }
        RuntimeObjectManager::NotifyDependencySource(GetObjectHandle(), DependencyObjectState::Modified);
    }

    gfx::TextureHandle CurveLinearColorAtlas::GetTextureHandle() const
    {
        return m_texHandle;
    }

    bool CurveLinearColorAtlas::CreateGPUResource()
    {
        if (m_isCreatedGpuResource)
        {
            return true;
        }
        m_isCreatedGpuResource = true;
        Generate();

        return true;
    }
    void CurveLinearColorAtlas::DestroyGPUResource()
    {
        if (!m_isCreatedGpuResource)
        {
            return;
        }
        if (m_texHandle.IsValid())
        {
            auto* renderThread = Application::GetRenderThread();
            renderThread->EnqueueUpdate_AnyThread(
                [h = m_texHandle](gfx::GFXResourceManager* mgr) { mgr->Destroy(h); });
            m_texHandle = gfx::TextureHandle{};
        }
        m_isCreatedGpuResource = false;
    }
    int32_t CurveLinearColorAtlas::GetWidth() const
    {
        return m_width;
    }
    int32_t CurveLinearColorAtlas::GetHeight() const
    {
        return m_height;
    }
    void CurveLinearColorAtlas::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == NAMEOF(m_colorCurveAssets))
        {
            RebuildObserver();
            // Generate();
        }
        Generate();
    }

    void CurveLinearColorAtlas::OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg)
    {
        base::OnNotifyObserver(inDependency, msg);
        if (msg == DependencyObjectState::Modified)
        {
            Generate();
        }

    }

    void CurveLinearColorAtlas::GetSubscribeObserverHandles(array_list<ObjectHandle>& out)
    {
        base::GetSubscribeObserverHandles(out);
        for (auto& curve : *m_colorCurveAssets)
        {
            out.push_back(curve.GetHandle());
        }
    }
    void CurveLinearColorAtlas::OnCollectAssetDependencies(array_list<guid_t>& deps)
    {
        base::OnCollectAssetDependencies(deps);
        if (m_colorCurveAssets)
        {
            for (auto& curve : *m_colorCurveAssets)
            {
                deps.push_back(curve.GetGuid());
            }
        }
    }
} // namespace pulsar