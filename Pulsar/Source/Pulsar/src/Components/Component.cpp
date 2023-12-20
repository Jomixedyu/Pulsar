#include "Components/Component.h"
#include <Pulsar/Components/Component.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Node.h>

namespace pulsar
{

    ObjectPtr<Node> Component::GetAttachedNode() const
    {
        return m_attachedNode;
    }
    ObjectPtr<Node> Component::GetOwnerNode() const
    {
        return m_ownerNode;
    }
    World* Component::GetWorld() const
    {
        return GetAttachedNode()->GetRuntimeWorld();
    }
    ObjectPtr<Scene> Component::GetRuntimeScene() const
    {
        return m_runtimeScene;
    }
    ObjectPtr<TransformComponent> Component::GetTransform() const
    {
        return GetAttachedNode()->GetTransform();
    }
    void Component::OnReceiveMessage(MessageId id)
    {
        if(id == MessageId_OnChangedTransform())
        {
            OnMsg_TransformChanged();
        }
    }
    bool Component::EqualsComponentType(Type* type)
    {
        return this->GetType() == type;
    }

    void Component::OnTick(Ticker ticker)
    {
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
        m_runtimeScene = GetAttachedNode()->GetRuntimeOwnerScene();
    }
    void Component::EndComponent()
    {
        m_beginning = false;
    }
} // namespace pulsar