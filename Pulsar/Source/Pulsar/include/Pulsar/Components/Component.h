#pragma once

#include "Pulsar/Gizmos.h"
#include "CoreLib.Serialization/ObjectSerializer.h"

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
    class GizmoPainter;

    using MessageId = size_t;

    class AbstractComponentAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::AbstractComponentAttribute, Attribute);
    };

    struct ComponentSerializer
    {
        ComponentSerializer(ser::VarientRef obj, bool isWrite, bool editorData)
            : Object(std::move(obj)),
              IsWrite(isWrite),
              HasEditorData(editorData)
        {
        }

        ComponentSerializer(const ComponentSerializer&) = delete;
        ComponentSerializer(ComponentSerializer&&) = delete;
    public:
        ser::VarientRef Object;
        hash_map<ObjectHandle, ObjectHandle>* MovingTable = nullptr;
        bool IsWrite;
        const bool HasEditorData;
    };

    class Component : public ObjectBase, public ITickable
    {
        friend class Node;
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Component, ObjectBase);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
    public:
        virtual void Serialize(ComponentSerializer* s);
        [[always_inline]] const ObjectPtr<Node>& GetNode() const noexcept { return m_ownerNode; }
        ObjectPtr<Node> GetMasterComponent() const;
        World* GetWorld() const;
        ObjectPtr<Scene> GetRuntimeScene() const;
        TransformComponent* GetTransform() const;
        array_list<ObjectHandle> GetReferenceHandles() const;
        void SendMessage(MessageId msgid);
        virtual BoxSphereBounds3f GetBoundsWS() { return {}; }
        virtual bool HasBounds() const { return false; }

        virtual void OnTransformChanged() {}

        bool CanDrawGizmo() const { return m_drawGizmo; }
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

    public:
        // ITickable interface
        void OnTick(Ticker ticker) override;
    public:
        Component();
    private:
        ObjectPtr<Component> m_masterComponent;
        ObjectPtr<Node> m_ownerNode;
        ObjectPtr<Scene> m_runtimeScene;
    protected:
        bool m_beginning = false;
        bool m_drawGizmo = false;
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
