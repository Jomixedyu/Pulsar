#pragma once

#include <CoreLib/CoreLib.h>
#include <Apatite/ObjectBase.h>
#include <Apatite/Math.h>

namespace apatite
{

    //namespace MessageType
    //{
    //    constexpr inline MessageType_t Enabled = 12;
    //    constexpr inline MessageType_t Disabled = 13;
    //}

    class Node;

    class Component : public ObjectBase
    {
        friend class Node;
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Component, ObjectBase);
        //SAPPHIRE_SCRIPTABLE_ASSET_GUID("223c87558d5e4910b72cf9f092f877e1");

    public:
        bool get_enabled() const { return enabled_; }
        void set_enabled(bool value);
        wptr<Node> get_node() { return node_; }
    public:
		virtual bool EqualsComponentType(Type* type);
        //virtual void SendMessage(MessageType_t msg);

        virtual void OnInitialize() {}
        virtual void OnTick() {}
        virtual void OnDestory() {}

        virtual void OnEnabled() {}
        virtual void OnDisabled() {}

    public:
        Component() {}
    private:
        wptr<Node> node_;
        bool enabled_ = true;
    };
    CORELIB_DECL_SHORTSPTR(Component);
}
