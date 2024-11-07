#pragma once
#include "GFXInclude.h"
#include "GFXTextureView.h"
#include "TextureClasses.h"
#include <cstdint>
#include <array>

namespace gfx
{
    struct GFXSamplerConfig
    {
        GFXSamplerFilter Filter;
        GFXSamplerAddressMode AddressMode;
    };

    struct GFXTextureCreateInfo
    {
        const uint8_t* imageData{};
        size_t dataLength{};
        int32_t width{};
        int32_t height{};
        int32_t depth{1};
        GFXTextureFormat format{};
        GFXTextureDataType dataType{};
        GFXSamplerConfig samplerCfg{};
        GFXTextureTargetType targetType{};
        uint32_t mipLevels{1};
        uint32_t arrayLayers{1};
    };

    class GFXTexture
    {
    public:
        virtual ~GFXTexture() = default;
        GFXTexture(int32_t width, int32_t height, int32_t depth, GFXSamplerConfig cfg)
            : m_width(width), m_height(height), m_depth(depth), m_samplerConfig(cfg)
        {
        }
    public:
        virtual int32_t GetWidth() const { return m_width; }
        virtual int32_t GetHeight() const { return m_height; }
        virtual int32_t GetDepth() const { return m_depth; }
        virtual size_t GetArrayCount() const { return m_arrayLayers; }
        virtual size_t GetMipLevels() const { return m_mipLevels; }
        virtual const type_info& GetClassId() const = 0;
        virtual GFXTextureTargetType GetTargetType() const = 0;
        virtual GFXTextureFormat GetFormat() const = 0;
        virtual GFXTexture2DView_sp Get2DView(size_t index = 0) = 0;

    public:
        std::array<float, 4> TargetClearColor;
    protected:
        GFXSamplerConfig m_samplerConfig{};
        int32_t m_width, m_height, m_depth;
        size_t m_mipLevels{1};
        size_t m_arrayLayers{1};
    };
    GFX_DECL_SPTR(GFXTexture);

} // namespace gfx