#include "Node.h"
#include "Scene.h"
#include "World.h"
#include <Components/RendererComponent.h>
#include <Pulsar/Components/Component.h>
#include <Pulsar/Logger.h>
#include <Pulsar/TransformUtil.h>
#include <stack>

using namespace std;

namespace pulsar
{
    bool Node::GetIsActive() const
    {
        Node_ref node = this->GetObjectHandle();
        while (node)
        {
            if (!node->GetIsActiveSelf())
            {
                return false;
            }
            if (auto parent = node->GetTransform()->GetParent())
            {
                node = parent->GetAttachedNode();
            }
            else
            {
                node = nullptr;
            }
        }
        return true;
    }
    void Node::SetIsActiveSelf(bool value)
    {
        if (m_active == value)
        {
            return;
        }
        m_active = value;
        if (value)
            OnActive();
        else
            OnInactive();
        for (auto child : *GetTransform()->GetChildren())
        {
            if (child->GetAttachedNode()->GetIsActiveSelf())
            {
                child->GetAttachedNode()->OnParentActiveChanged();
            }
        }
    }
    ObjectPtr<Node> Node::GetParent() const
    {
        if (auto p = GetTransform()->GetParent())
        {
            return p->GetAttachedNode();
        }
        return {};
    }
    void Node::OnActive()
    {
        for (auto& comp : *m_components)
        {
            BeginComponent(comp);
        }
    }
    void Node::OnInactive()
    {
        for (auto& comp : *m_components)
        {
            EndComponent(comp);
        }
    }

    void Node::OnParentActiveChanged()
    {
        if (m_runtimeScene)
        {
            if (GetIsActive())
            {
                OnActive();
            }
            else
            {
                OnInactive();
            }
        }
    }

    Node::Node()
    {
        this->m_components = mksptr(new List<ObjectPtr<Component>>);
    }

    void Node::OnConstruct()
    {
        this->AddComponent<TransformComponent>();
    }

    void Node::OnDestroy()
    {
        base::OnDestroy();

        for (auto& child : *GetTransform()->GetChildren())
        {
            DestroyObject(child->GetAttachedNode());
        }

        for (auto& comp : *m_components)
        {
            DestroyObject(comp);
        }
    }

    void Node::BeginNode(ObjectPtr<Scene> scene)
    {
        m_runtimeScene = scene;
        if (GetIsActive())
        {
            OnActive();
        }
    }
    void Node::EndNode()
    {
        if (GetIsActive())
        {
            OnInactive();
        }
        m_runtimeScene = nullptr;
    }

    ObjectPtr<Component> Node::AddComponent(Type* type)
    {
        Object_sp obj = type->CreateSharedInstance({});
        Component_sp component = sptr_cast<Component>(obj);
        component->Construct();

        // init
        component->m_attachedNode = self_ref();
        component->m_ownerNode = self_ref();
        this->m_components->push_back(component);

        if (m_runtimeScene && m_runtimeScene->GetWorld())
        {
            BeginComponent(component);
        }

        return component;
    }

    ObjectPtr<Component> Node::GetComponent(Type* type) const
    {
        for (auto& item : *this->m_components)
        {
            if (type->IsInstanceOfType(item.GetPtr()))
            {
                return item;
            }
        }
        return nullptr;
    }

    void Node::GetAllComponents(List_sp<ObjectPtr<Component>>& list)
    {
        for (auto& item : *this->m_components)
        {
            list->push_back(item);
        }
    }
    array_list<ObjectPtr<Component>> Node::GetAllComponentArray() const
    {
        return *this->m_components;
    }

    void Node::SendMessage(MessageId id)
    {
        for (auto& comp : *this->m_components)
        {
            comp->OnReceiveMessage(id);
        }
    }

    void Node::OnTick(Ticker ticker)
    {
        for (auto& comp : *this->m_components)
        {
            if (IsValid(comp))
            {
                comp->OnTick(ticker);
            }
        }
    }
    void Node::BeginComponent(Component_ref component)
    {
        component->BeginComponent();
        component->OnReceiveMessage(MessageId_OnChangedTransform());
    }

    void Node::EndComponent(Component_ref component)
    {
        if (component)
        {
            component->EndComponent();
        }
    }

    ObjectPtr<Node> Node::StaticCreate(string_view name, TransformComponent_ref parent, ObjectFlags flags)
    {
        Node_sp node = mksptr(new Node);
        node->SetObjectFlags(flags);
        node->SetName(name);
        node->Construct();
        node->GetTransform()->SetParent(parent);
        return node;
    }
    World* Node::GetRuntimeWorld() const
    {
        if (IsValid(m_runtimeScene))
        {
            return m_runtimeScene->GetWorld();
        }
        return nullptr;
    }
} // namespace pulsar
