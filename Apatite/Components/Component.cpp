#include <Apatite/Components/Component.h>
#include <Apatite/Node.h>

namespace apatite
{
    void Component::set_enabled(bool value)
    {
        if (this->enabled_ == value) return;
        this->enabled_ = value;
        if (value)
        {
            //this->SendMessage(MessageType::Enabled);
            this->OnEnabled();
        }
        else
        {
            //this->SendMessage(MessageType::Disabled);
            this->OnDisabled();
        }
    }

    bool Component::EqualsComponentType(Type* type)
    {
        return this->GetType() == type;
    }

    void Component::OnTick(Ticker ticker)
    {
    }

    //void Component::SendMessage(MessageType_t msg)
    //{
    //    switch (msg)
    //    {
    //    case MessageType::Initialize:
    //        this->OnInitialize(); break;
    //    case MessageType::Enabled:
    //        this->OnEnabled(); break;
    //    case MessageType::Disabled:
    //        this->OnDisabled(); break;
    //    case MessageType::Destory:
    //        this->OnDestory(); break;
    //    }
    //}

}