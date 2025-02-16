#pragma once
#include "Assets/RenderTextureCube.h"
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

        void PostEditChange(FieldInfo* info) override;

    public:
        void SetTexture(const RCPtr<RenderTextureCube>& tex);
        RCPtr<RenderTextureCube> GetTexture() const { return m_textureCube; }

    protected:
        RenderCapturePassInfo m_cube[6]{};

        CORELIB_REFL_DECL_FIELD(m_textureCube)
        RCPtr<RenderTextureCube> m_textureCube;
    };
}