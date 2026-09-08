#include <CoreLib/File.h>
#include <Pulsar/Application.h>
#include <Pulsar/Assets/Texture2D.h>
#include <Pulsar/Util/TextureCompressionUtil.h>
#include <Pulsar/Rendering/RenderThread.h>
#include <Pulsar/Rendering/TextureProxy.h>
#include <gfx/GFXImage.h>
#include <gfx/GFXResourceManager.h>

namespace pulsar
{
    Texture2D::Texture2D()
    {
    }
    Texture2D::~Texture2D() = default;

    template <typename T>
    void AssignEnum(T& e, const string& name)
    {
        uint32_t value{};
        Enum::StaticTryParse(cltypeof<get_boxing_type_t<T>>(), name, &value);
        e = (T)value;
    }

    void Texture2D::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (s->IsWrite)
        {
            assert(m_loadedOriginMemory);
            sser::ReadWriteStream(s->GetStream(), s->IsWrite, m_originMemory);

            s->Object->Add("IsCompressedNativeData", m_compressedOriginImage);

            auto size = s->Object->New(ser::VarientType::Object);
            size->Add("x", m_textureSize.x);
            size->Add("y", m_textureSize.y);
            s->Object->Add("Size", size);

            s->Object->Add("ChannelCount", m_channelCount);

            s->Object->Add("CompressedFormat", mkbox(m_compressionFormat)->GetName());
        }
        else // read
        {
            m_originMemory.clear();
            sser::ReadWriteStream(s->GetStream(), s->IsWrite, m_originMemory);
            m_compressedOriginImage = s->Object->At("IsCompressedNativeData")->AsBool();

            auto size = s->Object->At("Size");
            m_textureSize.x = size->At("x")->AsInt();
            m_textureSize.y = size->At("y")->AsInt();

            m_channelCount = s->Object->At("ChannelCount")->AsInt();

            auto compressedFormat = s->Object->At("CompressedFormat")->AsString();
            AssignEnum(m_compressionFormat, compressedFormat);

            m_loadedOriginMemory = true;
        }
    }

    void Texture2D::OnDestroy()
    {
        base::OnDestroy();
        this->DestroyGPUResource();
    }
    void Texture2D::FromNativeData(const uint8_t* data, size_t length, bool compressed, int width, int height, int channel)
    {
        m_originMemory.resize(length);
        std::memcpy(m_originMemory.data(), data, length);
        m_compressedOriginImage = compressed;
        m_textureSize.x = width;
        m_textureSize.y = height;
        m_channelCount = channel;
        m_loadedOriginMemory = true;
    }
    void Texture2D::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        auto name = info->GetName();
        if (name == NAMEOF(m_compressionFormat) ||
            name == NAMEOF(m_samplerFilter) ||
            name == NAMEOF(m_samplerAddressMode))
        {
            if (IsCreatedGPUResource())
            {
                DestroyGPUResource();
                CreateGPUResource();
            }
            SendOuterDependencyMsg(DependencyObjectState::Modified);
        }

    }

    static gfx::GFXTextureFormat _GetTextureFormat(TextureCompressionFormat format)
    {
        return Texture2D::StaticGetFormatMapping(OSPlatform::Windows64)->at(format);
    }

    bool Texture2D::CreateGPUResource()
    {
        if (IsCreatedGPUResource())
        {
            return true;
        }

        auto targetGfxFormat = _GetTextureFormat(m_compressionFormat);

        array_list<uint8_t> data{};
#ifdef WITH_EDITOR
        {
            array_list<uint8_t> uncompressedData;
            if (m_compressedOriginImage)
            {
                uncompressedData = gfx::LoadImageFromMemory(m_originMemory.data(), m_originMemory.size(),
                                                                   nullptr, nullptr, nullptr, m_channelCount);
            }
            else
            {
                uncompressedData = m_originMemory;
            }
            m_cachedUncompressedRawSize = uncompressedData.size();

            auto compressedData = TextureCompressionUtil::Compress(
                std::move(uncompressedData),
                m_textureSize.x,
                m_textureSize.y,
                m_channelCount,
                targetGfxFormat);
            data = std::move(compressedData);
        }
#else
        // TODO: runtime path - load from pre-baked native data
        // data = ...
#endif

        m_cachedNativeSize = data.size();

        m_isCreatedGPUResource = true;

        SamplerConfig samplerConfig;
        samplerConfig.Filter = GetSamplerFilter();
        samplerConfig.AddressMode = GetSamplerAddressMode();

        // 句柄分配线程安全，主线程立即拿到句柄；纹理创建与数据上传投递到渲染线程异步执行。
        // CreateGPUResource 只在游戏线程调用（由 Material::BuildRenderData 等驱动），
        // 投递创建保证渲染线程消费快照前资源已就绪。
        auto* renderThread = Application::GetRenderThread();
        m_proxy = std::make_shared<rendering::TextureProxy>(
            m_textureSize.x, m_textureSize.y, targetGfxFormat, samplerConfig, std::move(data));

        auto proxy = m_proxy;
        renderThread->EnqueueUpdate_AnyThread(
            [proxy = std::move(proxy)](gfx::GFXResourceManager*) mutable
            {
                proxy->OnCreateResource();
            });

        return true;
    }

    gfx::TextureHandle Texture2D::GetTextureHandle() const
    {
        return m_proxy ? m_proxy->GetTextureHandle() : gfx::TextureHandle{};
    }

    void Texture2D::DestroyGPUResource()
    {
        if (!IsCreatedGPUResource())
        {
            return;
        }
        m_isCreatedGPUResource = false;

        if (auto proxy = std::move(m_proxy))
        {
            Application::GetRenderThread()->EnqueueDestroy_AnyThread(
                [proxy = std::move(proxy)](gfx::GFXResourceManager*) mutable
                {
                    proxy->OnDestroyResource();
                });
        }
    }

    bool Texture2D::IsCreatedGPUResource() const
    {
        return m_isCreatedGPUResource;
    }

    bool Texture2D::IsSRGB() const
    {
        auto fmt = StaticGetFormatMapping(OSPlatform::Windows64)->at(m_compressionFormat);
        return fmt == gfx::GFXTextureFormat::BC3_SRGB || fmt == gfx::GFXTextureFormat::BC7_SRGB || fmt == gfx::GFXTextureFormat::R8G8B8A8_SRGB;
    }

} // namespace pulsar
