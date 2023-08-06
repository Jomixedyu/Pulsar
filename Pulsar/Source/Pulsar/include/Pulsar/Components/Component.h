#pragma once

#include <CoreLib/CoreLib.h>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/EngineMath.h>
#include <Pulsar/Ticker.h>

namespace pulsar
{
    class Node;
    
    using MessageId = size_t;

    class Component : public ObjectBase, public ITickable
    {
        friend class Node;
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::Component, ObjectBase);
    public:
        sptr<Node> GetAttachedNode();
        sptr<Node> GetOnwerNode();
        virtual bool get_is_tickable() const { return true; }
        virtual void OnReceiveMessage(MessageId id) {}
    public:
		virtual bool EqualsComponentType(Type* type);
    public:
        // Engine object lifecycle
        virtual void OnDestroy() override {}
        virtual void OnInitialize() {}
    public:        
        // ITickable interface
        virtual void OnTick(Ticker ticker) override;
    public:
        Component() {}
    private:
        wptr<Node> m_ownerNode;
        wptr<Node> m_attachedNode;

    };
    DECL_PTR(Component);
}
