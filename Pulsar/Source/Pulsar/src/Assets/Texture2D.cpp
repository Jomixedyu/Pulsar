#include <CoreLib/File.h>
#include <Pulsar/Application.h>
#include <Pulsar/Assets/Texture2D.h>
#include <Pulsar/Compressions/TextureCompression.h>

namespace pulsar
{
    Texture2D::Texture2D()
    {
    }
    Texture2D::~Texture2D() = default;

    void Texture2D::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (s->IsWrite)
        {
            if (m_loadedHostMemory)
            {
               sser::ReadWriteStream(s->Stream, s->IsWrite, m_nativeOriginalMemory);
            }
        }
        else //read
        {
            m_nativeOriginalMemory.clear();
            sser::ReadWriteStream(s->Stream, s->IsWrite, m_nativeOriginalMemory);
            m_loadedHostMemory = true;
        }
    }

    void Texture2D::OnDestroy()
    {
        base::OnDestroy();
        this->DestroyGPUResource();
    }
    void Texture2D::OnInstantiateAsset(AssetObject* obj)
    {

    }
    void Texture2D::LoadFromNativeData(const uint8_t* data, int32_t length)
    {
        m_nativeOriginalMemory.resize(length);
        std::memcpy(m_nativeOriginalMemory.data(), data, length);
    }
    void Texture2D::UnloadNativeData()
    {
        decltype(m_nativeOriginalMemory){}.swap(m_nativeOriginalMemory);
    }

    void Texture2D::LoadHostResource()
    {
        m_bakedDataMemory = std::move(m_nativeOriginalMemory);
        m_loadedHostMemory = true;
    }
    void Texture2D::UnloadHostResource()
    {
        decltype(m_bakedDataMemory)().swap(m_bakedDataMemory);
        m_loadedHostMemory = false;
    }

    bool Texture2D::CreateGPUResource()
    {
        if (IsCreatedGPUResource())
        {
            return true;
        }
        if (!m_loadedHostMemory)
        {
            LoadHostResource();
        }

        m_isCreatedGPUResource = true;

        m_tex = Application::GetGfxApp()->CreateTexture2DFromMemory(
            m_bakedDataMemory.data(),
            m_bakedDataMemory.size(),
            m_samplerConfig, m_enableReadWrite, m_isSrgb);

        if (m_loadedHostMemory)
        {
            UnloadHostResource();
        }
        return true;
    }

    void Texture2D::DestroyGPUResource()
    {
        if (!IsCreatedGPUResource())
        {
            return;
        }
        m_isCreatedGPUResource = false;
        m_tex.reset();
    }

    bool Texture2D::IsCreatedGPUResource() const
    {
        return m_isCreatedGPUResource;
    }

} // namespace pulsar
