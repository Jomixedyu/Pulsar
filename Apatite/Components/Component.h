#pragma once

#include <CoreLib/CoreLib.h>
#include <Apatite/ObjectBase.h>
#include <Apatite/Math.h>
#include <Apatite/Ticker.h>

namespace apatite
{

    //namespace MessageType
    //{
    //    constexpr inline MessageType_t Enabled = 12;
    //    constexpr inline MessageType_t Disabled = 13;
    //}

    class Node;

    class Component : public ObjectBase, public ITickable
    {
        friend class Node;
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Component, ObjectBase);
    public:
        bool get_enabled() const { return enabled_; }
        void set_enabled(bool value);
        wptr<Node> get_node() { return node_; }
    public:
		virtual bool EqualsComponentType(Type* type);

        virtual void OnDestroy() override {}

        virtual void OnEnabled() {}
        virtual void OnDisabled() {}

    public:
        Component() {}
    private:
        wptr<Node> node_;
        bool enabled_ = true;

        //ITickable
        virtual void OnTick(Ticker ticker) override;
    };
    CORELIB_DECL_SHORTSPTR(Component);
}
