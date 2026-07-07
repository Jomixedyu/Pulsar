#pragma once
#include "GFXResource.h"
#include "GFXTexture.h"

namespace gfx
{
    // A standalone GPU sampler state, decoupled from any texture. Created and
    // cached by GFXBuiltinResources keyed on GFXSamplerConfig; there are only a
    // handful of distinct sampler combinations in practice.
    class GFXSampler : public GFXResource
    {
    public:
        GFXResourceType GetResourceType() const override { return GFXResourceType::Sampler; }

        explicit GFXSampler(const GFXSamplerConfig& config) : m_config(config) {}

        const GFXSamplerConfig& GetConfig() const { return m_config; }

    protected:
        GFXSamplerConfig m_config{};
    };
    GFX_DECL_SPTR(GFXSampler);

} // namespace gfx
