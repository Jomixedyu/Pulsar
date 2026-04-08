#pragma once

#include "Pulsar/Gizmos.h"
#include <CoreLib.Serialization/ObjectSerializer.h>
#include <Pulsar/SceneObject.h>
#include <CoreLib/CoreLib.h>
#include <Pulsar/EngineMath.h>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/Ticker.h>

namespace pulsar
{
    class Node;
    class TransformComponent;
    class World;
    class Scene;
    class NodeCollection;
    class GizmoPainter;

    using MessageId = size_t;

    class AbstractComponentAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::AbstractComponentAttribute, Attribute);
    };
    class CategoryAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CategoryAttribute, Attribute);
    public:
        explicit CategoryAttribute(string_view category) : m_category(category)
        {
        }
        string_view GetCategory() const
        {
            return m_category;
        }
    private:
        string_view m_category;
    };

    class Component : public SceneObject, public ITickable
    {
        friend class Node;
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Component, SceneObject);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
        DECL_OBJECTPTR_SELF
    public:
        virtual void Serialize(SceneObjectSerializer* s);
        [[always_inline]] ObjectPtr<Node> GetNode() const noexcept { return m_ownerNode; }
        [[always_inline]] Node* GetNodePtr() const noexcept { return m_ownerNodePtr; }
        ObjectPtr<Component> GetMasterComponent() const;
        World* GetWorld() const;
        ObjectPtr<TransformComponent> GetTransform() const;
        void SendMessage(MessageId msgid);
        virtual BoxSphereBounds3f GetBoundsWS() { return {}; }
        virtual bool HasBounds() const { return false; }

        virtual void OnTransformChanged() {}

        bool CanDrawGizmo() const { return m_canDrawGizmo; }
        virtual void OnDrawGizmo(GizmoPainter* painter, bool selected) {}
    protected:
        virtual void OnReceiveMessage(MessageId id);
    public:
		virtual bool EqualsComponentType(Type* type);
    public:
        // Engine object lifecycle
        void OnDestroy() override {}

        virtual void BeginComponent();
        virtual void EndComponent();

        virtual void BeginPlay() {}
        virtual void EndPlay() {}

        virtual NodeCollection* GetOwnerNodeCollection() const override
        {
            return m_runtimeCollection;
        }
    public:
        // ITickable interface
        void OnTick(Ticker ticker) override;
    public:
        Component();
    private:
        ObjectPtr<Component> m_masterComponent;
        ObjectPtr<Node> m_ownerNode;
        Node* m_ownerNodePtr = nullptr;
        NodeCollection* m_runtimeCollection = nullptr;
    protected:
        bool m_beginning = false;
        bool m_canDrawGizmo = false;
    public:
        bool IsCollapsing = false;
    };
    DECL_PTR(Component);

    class ComponentInfoManager final
    {
    public:
        static string GetFriendlyComponentName(Type* type);
    };
}
