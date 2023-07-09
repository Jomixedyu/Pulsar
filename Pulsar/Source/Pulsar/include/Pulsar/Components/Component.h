#pragma once

#include <CoreLib/CoreLib.h>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/EngineMath.h>
#include <Pulsar/Ticker.h>

namespace pulsar
{
    class Node;

    class Component : public ObjectBase, public ITickable
    {
        friend class Node;
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::Component, ObjectBase);
    public:
        sptr<Node> get_node();
        virtual bool get_is_tickable() const { return true; }
    public:
		virtual bool EqualsComponentType(Type* type);
    public:
        virtual void OnDraw() {}
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
        wptr<Node> node_;

    };
    CORELIB_DECL_SHORTSPTR(Component);
}
