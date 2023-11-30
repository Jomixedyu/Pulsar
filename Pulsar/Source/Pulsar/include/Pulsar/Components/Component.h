#pragma once

#include <CoreLib/CoreLib.h>
#include <Pulsar/EngineMath.h>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/Ticker.h>

namespace pulsar
{
    class Node;
    class World;

    using MessageId = size_t;

    class Component : public ObjectBase, public ITickable
    {
        friend class Node;
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Component, ObjectBase);
    public:
        ObjectPtr<Node> GetAttachedNode() const;
        ObjectPtr<Node> GetOwnerNode() const;
        World* GetWorld() const;
        virtual bool get_is_tickable() const { return true; }
        virtual void OnReceiveMessage(MessageId id) {}
    public:
		virtual bool EqualsComponentType(Type* type);
    public:
        // Engine object lifecycle
        void OnDestroy() override {}

        virtual void BeginComponent();
        virtual void EndComponent() {}

        virtual void BeginPlay() {}
        virtual void EndPlay() {}
    public:        
        // ITickable interface
        void OnTick(Ticker ticker) override;
    public:
        Component() = default;
    private:
        ObjectPtr<Node> m_ownerNode;
        ObjectPtr<Node> m_attachedNode;
    public:
        bool IsCollapsing = false;
    };
    DECL_PTR(Component);
}
