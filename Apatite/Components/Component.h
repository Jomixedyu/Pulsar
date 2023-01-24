#pragma once

#include <CoreLib/CoreLib.h>
#include <Apatite/ObjectBase.h>
#include <Apatite/Math.h>
#include <Apatite/Ticker.h>

namespace apatite
{
    class Node;

    class Component : public ObjectBase, public ITickable
    {
        friend class Node;
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Component, ObjectBase);
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
