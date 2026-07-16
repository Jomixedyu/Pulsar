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
        // TODO: cube / 2DArray fallbacks are deferred until the vk backend gains
        // cube/array texture+view support (lands with set1 perPass wiring). Until
        // then every dimension falls back to the black 2D view; set0 materials only
        // use Texture2D, so this stopgap is never hit by a non-2D binding today.
        switch (binding.m_viewDimension)
        {
        case gfx::GFXTextureDataType::Texture2D:
        default:
            return builtin.GetBlackTexture2DView();
        }
    }
}
