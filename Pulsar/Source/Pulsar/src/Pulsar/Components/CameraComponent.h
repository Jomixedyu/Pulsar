#pragma once
#include <Pulsar/IconsForkAwesome.h>
#include "Component.h"
#include "Pulsar/Raycast.h"
#include "SceneCapture2DComponent.h"
#include "SceneCaptureComponent.h"
#include "gfx/GFXBuffer.h"
#include "gfx/GFXDescriptorSet.h"
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/RenderTexture.h>
#include <Pulsar/Meta/ToolFunctionAttribute.h>

namespace pulsar
{

    class CameraComponent : public SceneCapture2DComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CameraComponent, SceneCapture2DComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Capture"), new ComponentIconAttribute(ICON_FK_CAMERA));
        DECL_OBJECTPTR_SELF
    public:
        CameraComponent();
        ~CameraComponent() override;
        void Render();
    public:

        void Render(array_list<RenderCapturePassInfo*>& passes) override;

        void BeginComponent() override;
        void EndComponent() override;

        void PostEditChange(FieldInfo* info) override;
        void OnDrawGizmo(GizmoPainter* painter, bool selected) override;
        void OnTransformChanged() override;

        void ResizeManagedRenderTexture(int width, int height);
    public:

        void SetRenderTexture(const RCPtr<RenderTexture>& value, bool managed = false);

        Ray ScreenPointToRay(Vector2f mousePosition) const;

        bool IsGizmoPassEnabled() const { return m_gizmoPassEnabled; }
        void SetGizmoPassEnabled(bool value) { m_gizmoPassEnabled = value; }

        CORELIB_REFL_DECL_METHOD(ResetToDefault, new ToolFunctionAttribute("Reset to Default"));
        void ResetToDefault();

    protected:
        void BeginRT();
        void UpdateRT();
    protected:
        bool m_gizmoPassEnabled = true;

    };
    DECL_PTR(CameraComponent);
}
