#include <CoreLib/File.h>
#include <Pulsar/Application.h>
#include <Pulsar/Assets/Texture2D.h>

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
               sser::ReadWriteStream(s->Stream, s->IsWrite, m_nativeHostMemory);
            }
        }
        else //read
        {
            m_nativeHostMemory.clear();
            sser::ReadWriteStream(s->Stream, s->IsWrite, m_nativeHostMemory);
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
        Texture2D* tex = static_cast<Texture2D*>(obj);
    }

    void Texture2D::LoadHostResource(
        const uint8_t* data, int32_t length,
        const SamplerConfig& samplerConfig,
        bool enableReadWrite, TextureFormat format)
    {
        m_nativeHostMemory.resize(length);
        std::memcpy(m_nativeHostMemory.data(), data, length);
        m_samplerConfig = samplerConfig;
        m_enableReadWrite = enableReadWrite;
        m_format = format;
        m_loadedHostMemory = true;
    }
    void Texture2D::UnloadHostResource()
    {
        decltype(m_nativeHostMemory)().swap(m_nativeHostMemory);
        m_loadedHostMemory = false;
    }

    bool Texture2D::CreateGPUResource()
    {
        if (IsCreatedGPUResource())
        {
            return true;
        }
        m_isCreatedGPUResource = true;

        m_tex = Application::GetGfxApp()->CreateTexture2DFromMemory(
            m_nativeHostMemory.data(),
            m_nativeHostMemory.size(),
            m_samplerConfig, m_enableReadWrite, m_format);
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
