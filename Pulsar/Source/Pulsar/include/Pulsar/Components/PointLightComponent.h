#pragma once
#include "LightComponent.h"
#include "Pulsar/Rendering/RenderObject.h"

namespace pulsar
{
    class PointLightComponent : public LightComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::PointLightComponent, LightComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Light"));
    public:
        void BeginComponent() override;
        void EndComponent() override;

    public:
        BoxSphereBounds3f GetBoundsWS() override;
    public:
        float GetRadius() const { return m_radius; }
        void SetRadius(float value);
    protected:
        void OnDrawGizmo(GizmoPainter* painter, bool selected) override;
        void OnTransformChanged() override;

        void OnRadiusChanged();
        void PostEditChange(FieldInfo* info) override;

        CORELIB_REFL_DECL_FIELD(m_radius);
        float m_radius = 1;
    };
}