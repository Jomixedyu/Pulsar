#pragma once

#include "Component.h"

namespace pulsar
{
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

        void SetPosition(Vector3f value) { m_position = value; m_isDirtyMatrix = true; BroadcastChange(); }
        Vector3f GetPosition() const { return m_position; }
        Vector3f GetWorldPosition() const;
        void SetWorldPosition(Vector3f value);
        void Translate(Vector3f value);

        void SetScale(Vector3f value) { m_scale = value; m_isDirtyMatrix = true; BroadcastChange(); }
        Vector3f GetScale() const { return m_scale; }
        Vector3f GetWorldScale() const;

        void SetRotation(Quat4f rotation) { m_rotation = rotation; m_isDirtyMatrix = true; BroadcastChange(); }
        Quat4f GetRotation() const { return m_rotation; }
        Vector3f GetEuler() const;
        void SetEuler(Vector3f value);

        TransformComponent();
        void OnTick(Ticker ticker) override;

        const Matrix4f& GetLocalToWorldMatrix() const { return m_localToWorldMatrix; }
        Matrix4f GetChildLocalToWorldMatrix() const;
        const Matrix4f& GetWorldToLocalMatrix() const { return m_worldToLocalMatrix; }
    protected:
        void RebuildLocalToWorldMatrix();
        void PostEditChange(FieldInfo* info) override;
        void BroadcastChange();
    protected:
        CORELIB_REFL_DECL_FIELD(m_localToWorldMatrix, new ReadOnlyPropertyAttribute);
        Matrix4f m_localToWorldMatrix;
        Matrix4f m_worldToLocalMatrix;
        bool m_isDirtyMatrix = false;

        CORELIB_REFL_DECL_FIELD(m_position);
        Vector3f m_position{};
        CORELIB_REFL_DECL_FIELD(m_euler);
        Vector3f m_euler{};
        CORELIB_REFL_DECL_FIELD(m_scale);
        Vector3f m_scale;
        CORELIB_REFL_DECL_FIELD(m_rotation);
        Quat4f m_rotation{};

        CORELIB_REFL_DECL_FIELD(m_children, new HidePropertyAttribute);
        List_sp<ObjectPtr<TransformComponent>> m_children{};

        CORELIB_REFL_DECL_FIELD(m_parent, new HidePropertyAttribute);
        ObjectPtr<TransformComponent> m_parent{};
    };
    DECL_PTR(TransformComponent);
}