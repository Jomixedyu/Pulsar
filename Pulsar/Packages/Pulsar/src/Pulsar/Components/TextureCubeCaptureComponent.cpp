#include "Components/TextureCubeCaptureComponent.h"

namespace pulsar
{

    void TextureCubeCaptureComponent::BeginComponent()
    {
        base::BeginComponent();

    }
    void TextureCubeCaptureComponent::EndComponent()
    {
        base::EndComponent();

    }
    void TextureCubeCaptureComponent::Render(array_list<RenderCapturePassInfo*>& passes)
    {
        base::Render(passes);

        for (auto& cube : m_cube)
        {
            passes.push_back(&cube);
        }
    }

    void TextureCubeCaptureComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == NAMEOF(m_textureCube))
        {

        }
    }
    void TextureCubeCaptureComponent::SetTexture(const RCPtr<RenderTextureCube>& tex)
    {
        m_textureCube = tex;
    }
} // namespace pulsar