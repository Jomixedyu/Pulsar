#include "DescriptorSetAssembler.h"

#include <Pulsar/Application.h>
#include <gfx/GFXApplication.h>
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

    gfx::GFXDescriptorSetLayout_sp BuildLayout(const ShaderPropertySetLayout* setLayout)
    {
        std::vector<gfx::GFXDescriptorLayoutDesc> descs;
        if (setLayout)
        {
            for (const auto& b : setLayout->m_bindings)
            {
                gfx::GFXDescriptorLayoutDesc desc{};
                desc.Type = b.m_type;
                desc.Stage = (b.m_stageFlags != gfx::GFXGpuProgramStageFlags::None)
                    ? b.m_stageFlags
                    : gfx::GFXGpuProgramStageFlags::VertexFragment;
                desc.BindingPoint = b.m_bindingPoint;
                descs.push_back(desc);
            }
        }
        return Application::GetGfxApp()->GetOrCreateDescriptorSetLayout(descs.data(), static_cast<uint32_t>(descs.size()));
    }
}
