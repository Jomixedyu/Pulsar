#include "CameraComponent.h"
#include <Apatite/Private/RenderInterface.h>

namespace apatite
{
    void CameraComponent::Render()
    {
        if (IsValid(this->render_target))
        {
            assert(this->render_target->GetIsBindGPU());

            RenderTextureRAII rt{ this->render_target };
            {
                auto [r, g, b, a] = this->backgroundColor;
                detail::RenderInterface::Clear(r, g, b, a);
                //draw ...
            }
        }
    }
}
