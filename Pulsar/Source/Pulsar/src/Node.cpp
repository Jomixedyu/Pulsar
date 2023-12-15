#include "Node.h"
#include <Pulsar/Components/Component.h>
#include <Pulsar/TransformUtil.h>
#include <stack>
#include <Pulsar/Logger.h>
#include "Scene.h"
#include <Components/RendererComponent.h>
#include "World.h"

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
        for (auto& comp : *m_components)
        {
            BeginComponent(comp);
        }
    }
    void Node::EndNode()
    {
        for (auto& comp : *m_components)
        {
            EndComponent(comp);
        }
        m_runtimeScene = nullptr;
    }

    ObjectPtr<Component> Node::AddComponent(Type* type)
    {
        Object_sp obj = type->CreateSharedInstance({});
        Component_sp component = sptr_cast<Component>(obj);
        component->Construct();

        //init
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

        //if (component->GetType()->IsImplementedInterface(cltypeof<IRendererComponent>()))
        //{
        //    if (auto ro = interface_cast<IRendererComponent>(component.GetPtr())->CreateRenderObject())
        //    {
        //        if (auto world = GetRuntimeWorld())
        //        {
        //            world->AddRenderObject(ro);
        //        }
        //    }
        //}

        component->BeginComponent();
    }

    void Node::EndComponent(Component_ref component)
    {
        if(component)
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
}

