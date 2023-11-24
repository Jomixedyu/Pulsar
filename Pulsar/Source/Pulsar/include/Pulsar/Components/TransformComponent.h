#pragma once

#include "Component.h"

namespace pulsar
{
    class HideInComponentPropertyAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::HideInComponentPropertyAttribute, Attribute);
    };

    class TransformComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::TransformComponent, Component);
    public:
        
        ObjectPtr<TransformComponent> FindByName(string_view name) const;
        ObjectPtr<TransformComponent> FindByPath(string_view path) const;
        ObjectPtr<TransformComponent> GetParent() const { return m_parent; }
        void SetParent(ObjectPtr<TransformComponent> parent);

        const List_sp<ObjectPtr<TransformComponent>>& GetChildren() const { return m_children; }
        size_t GetChildCount() const { return m_children->size(); }
        ObjectPtr<TransformComponent> GetChild(int32_t index) const { return m_children->at(index); }

        void SetPosition(Vector3f value) { m_position = value; }
        Vector3f GetPosition() const { return m_position; }
        void SetScale(Vector3f value) { m_scale = value; }
        Vector3f GetScale() const { return m_scale; }

        TransformComponent();
    protected:
        CORELIB_REFL_DECL_FIELD(m_position);
        Vector3f m_position;
        CORELIB_REFL_DECL_FIELD(m_euler);
        Vector3f m_euler;
        CORELIB_REFL_DECL_FIELD(m_scale);
        Vector3f m_scale;

        Quat4f m_rotation;

        CORELIB_REFL_DECL_FIELD(m_children, new HideInComponentPropertyAttribute);
        List_sp<ObjectPtr<TransformComponent>> m_children;

        CORELIB_REFL_DECL_FIELD(m_parent, new HideInComponentPropertyAttribute);
        ObjectPtr<TransformComponent> m_parent;
    };
    DECL_PTR(TransformComponent);
}