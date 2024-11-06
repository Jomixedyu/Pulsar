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
    void TextureCubeCaptureComponent::SetTexture(const RCPtr<TextureCube>& tex)
    {
        m_textureCube = tex;
    }
} // namespace pulsar