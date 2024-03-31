#include "Components/Component.h"

#include "CoreLib.Serialization/JsonSerializer.h"

#include <Pulsar/Components/Component.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/Node.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/World.h>

namespace pulsar
{

    void Component::Serialize(ComponentSerializer* s)
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

    ObjectPtr<Node> Component::GetMasterComponent() const
    {
        return m_masterComponent;
    }
    World* Component::GetWorld() const
    {
        return GetNode()->GetRuntimeWorld();
    }
    ObjectPtr<Scene> Component::GetRuntimeScene() const
    {
        return m_runtimeScene;
    }
    TransformComponent* Component::GetTransform() const
    {
        return m_ownerNode->GetTransform();
    }
    array_list<ObjectHandle> Component::GetReferenceHandles() const
    {
        array_list<ObjectHandle> handles;
        for (auto fieldInfo : this->GetType()->GetFieldInfos(TypeBinding::NonPublic))
        {
            if(fieldInfo->GetWrapType() == cltypeof<BoxingObjectPtrBase>())
            {
                auto value = fieldInfo->GetValue(this);
                auto id = UnboxUtil::Unbox<ObjectPtrBase>(value);
                handles.push_back(id.GetHandle());
            }
        }
        return handles;
    }
    void Component::OnReceiveMessage(MessageId id)
    {
        if (id == MessageId_OnChangedTransform())
        {
            OnMsg_TransformChanged();
        }
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
        m_runtimeScene = GetNode()->GetRuntimeOwnerScene();
    }
    void Component::EndComponent()
    {
        m_beginning = false;
    }
} // namespace pulsar