#pragma once
#include "GFXResource.h"
#include "GFXTexture.h"

namespace gfx
{
    class GFXResourceRegistry;

    // A standalone GPU sampler state, decoupled from any texture. Created and
    // cached by GFXBuiltinResources keyed on GFXSamplerConfig; there are only a
    // handful of distinct sampler combinations in practice.
    class GFXSampler : public GFXResource
    {
    public:
        GFXResourceType GetResourceType() const override { return GFXResourceType::Sampler; }

        const GFXSamplerConfig& GetConfig() const { return m_config; }

    protected:
        explicit GFXSampler(GFXResourceRegistry* registry, const GFXSamplerConfig& config)
            : GFXResource(registry), m_config(config) {}

        GFXSamplerConfig m_config{};
    };
    GFX_DECL_PTR(GFXSampler);

} // namespace gfx
