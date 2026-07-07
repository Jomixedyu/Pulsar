#include "DescriptorSetAssembler.h"

#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXBuffer.h>
#include <gfx/GFXTextureView.h>

namespace pulsar::DescriptorSetAssembler
{
    void Write(gfx::GFXDescriptorSet* set,
               const ShaderPropertySetLayout& setLayout,
               const RenderResourceRegistry& reg)
    {
        if (!set)
            return;

        for (const auto& b : setLayout.m_bindings)
        {
            gfx::GFXResource* r = reg.Resolve(b);
            if (!r)
                continue; // unsupported binding type

            auto* desc = set->FindByBinding(b.m_bindingPoint);
            if (!desc)
                desc = set->AddDescriptor(b.m_name, b.m_bindingPoint);

            switch (b.m_type)
            {
            case gfx::GFXDescriptorType::ConstantBuffer:
                desc->SetConstantBuffer(static_cast<gfx::GFXBuffer*>(r));
                break;
            case gfx::GFXDescriptorType::ConstantBufferDynamic:
                desc->SetConstantBufferDynamic(static_cast<gfx::GFXBuffer*>(r));
                break;
            case gfx::GFXDescriptorType::StructuredBuffer:
                desc->SetStructuredBuffer(static_cast<gfx::GFXBuffer*>(r));
                break;
            case gfx::GFXDescriptorType::CombinedImageSampler:
            case gfx::GFXDescriptorType::Texture2D:
                desc->SetTextureSampler2D(static_cast<gfx::GFXTexture2DView*>(r));
                break;
            default:
                break;
            }
        }

        set->Submit();
    }
}
