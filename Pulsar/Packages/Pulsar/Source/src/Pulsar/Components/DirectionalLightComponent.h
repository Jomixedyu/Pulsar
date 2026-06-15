#pragma once
#include <Pulsar/IconsForkAwesome.h>
#include "LightComponent.h"
#include "Pulsar/Rendering/RenderObject.h"
#include "Pulsar/Scene.h"

namespace pulsar
{
    class DirectionalLightComponent : public LightComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::DirectionalLightComponent, LightComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Light"), new ComponentIconAttribute(ICON_FK_SUN_O));
    public:
        void BeginComponent() override;
        void EndComponent() override;


    protected:
        SPtr<rendering::RenderProxy> CreateRenderProxy() override;
        void SyncRenderProxy() override;

        void PostEditChange(FieldInfo* info) override;

        void OnIntensityChanged() override;
        void OnLightColorChanged() override;

        void OnTransformChanged() override;
        SPtr<rendering::RenderObject> m_gizmos;

        Vector3f m_vector{};
    };
}