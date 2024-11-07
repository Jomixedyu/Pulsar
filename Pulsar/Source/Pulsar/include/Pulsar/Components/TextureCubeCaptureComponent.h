#pragma once
#include "Assets/TextureCube.h"
#include "SceneCaptureComponent.h"

namespace pulsar
{
    class TextureCubeCaptureComponent : public SceneCaptureComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::TextureCubeCaptureComponent, SceneCaptureComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Capture"));
    public:

        void BeginComponent() override;
        void EndComponent() override;

        void Render(array_list<RenderCapturePassInfo*>& passes) override;

        void SetTexture(const RCPtr<TextureCube>& tex);
        RCPtr<TextureCube> GetTexture() const { return m_textureCube; }

    protected:
        RenderCapturePassInfo m_cube[6]{};
        RCPtr<TextureCube> m_textureCube;
    };
}