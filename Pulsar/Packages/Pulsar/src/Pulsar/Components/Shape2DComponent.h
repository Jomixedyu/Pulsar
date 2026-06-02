#pragma once
#include "Component.h"

namespace pulsar
{
    enum class Shape2DType
    {
        Box,
        Circle,
        Capsule
    };
    class Shape2DComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Shape2DComponent, Component);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
    public:
        Shape2DComponent();

        virtual Shape2DType GetShapeType() = 0;

        Vector2f GetSize() const { return m_size; }
        float GetRadius() const { return m_radius; }

    public:
        CORELIB_REFL_DECL_FIELD(m_offset);
        Vector2f m_offset{};
        CORELIB_REFL_DECL_FIELD(m_size);
        Vector2f m_size{0.5f, 0.5f};
        CORELIB_REFL_DECL_FIELD(m_radius);
        float m_radius = 0.5f;

        CORELIB_REFL_DECL_FIELD(m_density);
        float m_density = 1.f;
        CORELIB_REFL_DECL_FIELD(m_friction);
        float m_friction = 0.6f;
        CORELIB_REFL_DECL_FIELD(m_isSensor);
        bool m_isSensor = false;
    };
    DECL_PTR(Shape2DComponent);
} // namespace pulsar