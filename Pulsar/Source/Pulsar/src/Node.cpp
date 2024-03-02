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
    void Node::Serialize(NodeSerializer* s)
    {
        if (s->IsWrite)
        {
            s->Object->Add("Name", GetName());
            s->Object->Add("IsActive", m_active);

            auto componentsObj = s->Object->New(ser::VarientType::Array);
            for (auto& component : *m_components)
            {
                auto compObj = s->Object->New(ser::VarientType::Object);
                ComponentSerializer componentSerializer{compObj, s->IsWrite, s->HasEditorData};
                component->Serialize(&componentSerializer);
                componentsObj->Push(compObj);
            }
            s->Object->Add("Components", componentsObj);

        }
    }
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

        for (auto child : *GetTransform()->GetChildren())
        {
            if (child->GetAttachedNode()->GetIsActiveSelf())
            {
                child->GetAttachedNode()->OnParentActiveChanged();
            }
        }
        if (value)
            OnActive();
        else
            OnInactive();
    }
    ObjectPtr<Node> Node::GetParent() const
    {
        if (auto p = GetTransform()->GetParent())
        {
            return p->GetAttachedNode();
        }
        return {};
    }
    void Node::SetParent(ObjectPtr<Node> parent)
    {
        GetTransform()->SetParent(parent->GetTransform());
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
    TransformComponent_ref Node::GetTransform() const
    {
        return ref_cast<TransformComponent>(m_components->at(0));
    }

    Node::Node()
    {
        m_flags |= OF_LifecycleManaged;
        this->m_components = mksptr(new List<ObjectPtr<Component>>);
    }
    Node::~Node()
    {

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
            DestroyObject(comp, true);
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

        if (!m_components->empty() && type->IsSubclassOf(cltypeof<TransformComponent>()))
        {
            DestroyObject(m_components->at(0), true);
            m_components->at(0) = component;
        }
        else
        {
            this->m_components->push_back(component);
        }

        if (m_runtimeScene && m_runtimeScene->GetWorld())
        {
            BeginComponent(component);
            component->SendMessage(MessageId_OnChangedTransform());
        }

        return component;
    }
    void Node::DestroyComponent(ObjectPtr<Component> component)
    {
        auto it = std::ranges::find(*m_components, component);
        if (it == m_components->end())
        {
            return;
        }
        if (m_runtimeScene)
        {
            EndComponent(component);
        }
        DestroyObject(component, true);
        m_components->erase(it);
    }
    int Node::IndexOf(ObjectPtr<Component> component) const
    {
        const auto count = GetComponentCount();
        for (int i = 0; i < count; ++i)
        {
            if (m_components->at(i) == component)
            {
                return i;
            }
        }
        return -1;
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

    void Node::GetAllComponents(array_list<ObjectPtr<Component>>& list)
    {
        for (auto& item : *this->m_components)
        {
            list.push_back(item);
        }
    }
    const array_list<ObjectPtr<Component>>& Node::GetAllComponentArray() const
    {
        return *this->m_components;
    }

    void Node::SendMessage(MessageId id)
    {
        if (m_runtimeScene == nullptr)
        {
            return;
        }
        for (auto& comp : *this->m_components)
        {
            comp->SendMessage(id);
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
    }

    void Node::EndComponent(Component_ref component)
    {
        if (component)
        {
            component->EndComponent();
        }
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
