#include "Components/Component.h"

#include "CoreLib.Serialization/JsonSerializer.h"

#include <Pulsar/Components/Component.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/Node.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/World.h>
#include "Pulsar/Scene.h"

namespace pulsar
{

    void Component::Serialize(SceneObjectSerializer* s)
    {
        if (s->IsWrite)
        {
            auto json = ser::JsonSerializer::Serialize(this, {});
            s->Object->AssignParse(json);
        }
        else // read
        {
            ser::JsonSerializer::Deserialize(s->Object->ToString(false), GetType(), self());
        }
    }

    ObjectPtr<Component> Component::GetMasterComponent() const
    {
        return m_masterComponent;
    }
    World* Component::GetWorld() const
    {
        return GetNode()->GetRuntimeWorld();
    }
    ObjectPtr<TransformComponent> Component::GetTransform() const
    {
        return m_ownerNode->GetTransform();
    }

    void Component::OnReceiveMessage(MessageId id)
    {

    }
    void Component::SendMessage(MessageId msgid)
    {
        OnReceiveMessage(msgid);
    }
    bool Component::EqualsComponentType(Type* type)
    {
        return this->GetType() == type;
    }

    void Component::OnTick(Ticker ticker)
    {
    }
    Component::Component()
    {
        m_flags |= OF_LifecycleManaged;
    }
    string ComponentInfoManager::GetFriendlyComponentName(Type* type)
    {
        string name = type->GetShortName();

        static char com_str[] = "Component";
        if (name.ends_with("Component"))
        {
            name = name.substr(0, name.size() - sizeof(com_str) + 1);
        }
        return StringUtil::FriendlyName(name);
    }

    void Component::BeginComponent()
    {
        m_beginning = true;
        m_runtimeCollection = GetNode()->GetOwnerNodeCollection();

        if (m_canDrawGizmo)
        {
            GetWorld()->GetGizmosManager().AddGizmoComponent(self_ptr());
        }
    }
    void Component::EndComponent()
    {
        m_beginning = false;

        if (m_canDrawGizmo)
        {
            GetWorld()->GetGizmosManager().RemoveGizmoComponent(self_ptr());
        }
    }
} // namespace pulsar