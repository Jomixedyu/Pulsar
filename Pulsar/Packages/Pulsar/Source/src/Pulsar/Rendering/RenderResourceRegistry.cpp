#include "RenderResourceRegistry.h"
#include "ShaderPropertyLayout.h"

#include "Application.h"

#include <gfx/GFXApplication.h>

namespace pulsar
{
    gfx::GFXResource* RenderResourceRegistry::Resolve(const DescriptorBinding& binding) const
    {
        if (auto* r = Find(binding.m_name))
            return r;

        // Tier 3: engine-wide gfx builtin fallback, keyed by binding type / dimension.
        auto& builtin = Application::GetGfxApp()->GetBuiltinResources();

        if (binding.IsBuffer())
            return builtin.GetZeroBuffer();

        // Texture / combined-image-sampler: black view of the matching dimension.
        switch (binding.m_viewDimension)
        {
        case gfx::GFXTextureDataType::Texture2D:
        default:
            return builtin.GetBlackTexture2DView();
        }
    }
}
