#pragma once
#include "Component.h"
#include "Pulsar/HitResult.h"
#include "SceneCapture2DComponent.h"
#include "SceneCaptureComponent.h"
#include "gfx/GFXBuffer.h"
#include "gfx/GFXDescriptorSet.h"
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/RenderTexture.h>

namespace pulsar
{

    class CameraComponent : public SceneCapture2DComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CameraComponent, SceneCapture2DComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Capture"));
    public:
        CameraComponent();
        ~CameraComponent() override;
        void Render();
    public:

        void Render(array_list<RenderCapturePassInfo*>& passes) override;

        void BeginComponent() override;
        void EndComponent() override;

        void PostEditChange(FieldInfo* info) override;

        void ResizeManagedRenderTexture(int width, int height);
    public:

        const RCPtr<RenderTexture>&  GetRenderTexture() const { return m_renderTarget; }
        void                         SetRenderTexture(const RCPtr<RenderTexture>& value, bool managed = false);

        Ray ScreenPointToRay(Vector2f mousePosition) const;

    protected:
        void BeginRT();
        void UpdateRT();
    protected:

    };
    DECL_PTR(CameraComponent);
}