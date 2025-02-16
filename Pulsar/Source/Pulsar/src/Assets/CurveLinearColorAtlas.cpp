#include "Pulsar/Assets/CurveLinearColorAtlas.h"

#include "Application.h"

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
                list->Push(element.Handle.to_string());
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
                auto handleStr = curves->At(i)->AsString();
                auto handle = ObjectHandle::parse(handleStr);
                m_colorCurveAssets->push_back(handle);
            }
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
            m_gfxTexture = Application::GetGfxApp()->CreateTexture2DFromMemory((uint8_t*)m_bitmap.data(), bitmapDataSize, m_width, m_height, gfx::GFXTextureFormat::R8G8B8A8_UNorm, {});
        }
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
        m_gfxTexture.reset();
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
            Generate();
        }
        if (info->GetName() == NAMEOF(m_width))
        {
            Generate();
        }
        if (info->GetName() == NAMEOF(m_height))
        {
            Generate();
        }
    }
} // namespace pulsar