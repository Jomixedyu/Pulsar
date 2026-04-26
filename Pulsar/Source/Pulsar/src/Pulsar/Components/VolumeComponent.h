#pragma once
#include "Component.h"
#include "Pulsar/Assets/VolumeProfile.h"

namespace pulsar
{
    class VolumeComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::VolumeComponent, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Rendering"));

    public:
        VolumeComponent();

        void BeginComponent() override;
        void EndComponent() override;

        bool IsGlobal() const { return m_isGlobal; }
        void SetIsGlobal(bool value) { m_isGlobal = value; }

        float GetBlendDistance() const { return m_blendDistance; }
        void SetBlendDistance(float value) { m_blendDistance = value; }

        int GetPriority() const { return m_priority; }
        void SetPriority(int value) { m_priority = value; }

        const RCPtr<VolumeProfile>& GetProfile() const { return m_profile; }
        void SetProfile(const RCPtr<VolumeProfile>& profile) { m_profile = profile; }

        // Returns the effective box extent from the attached Node's Transform scale.
        // If the Node has no Transform, returns Vector3f::one().
        Vector3f GetEffectiveExtent() const;

        // Computes the blended weight for a given world position.
        // Returns 1.0 for global volumes, or a 0~1 blend based on distance to box bounds.
        float ComputeBlendWeight(const Vector3f& worldPos) const;

        void OnDrawGizmo(GizmoPainter* painter, bool selected) override;

    protected:
        CORELIB_REFL_DECL_FIELD(m_isGlobal);
        bool m_isGlobal = false;

        CORELIB_REFL_DECL_FIELD(m_blendDistance);
        float m_blendDistance = 0.0f;

        CORELIB_REFL_DECL_FIELD(m_priority);
        int m_priority = 0;

        CORELIB_REFL_DECL_FIELD(m_profile);
        RCPtr<VolumeProfile> m_profile;


    };

} // namespace pulsar
