#include "Node.h"
#include "Scene.h"
#include "World.h"
#include <Components/RendererComponent.h>
#include <Pulsar/Components/Component.h>
#include <Pulsar/Logger.h>
#include <Pulsar/TransformUtil.h>
#include <stack>
#include <utility>

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
                // add component description
                auto compObj = s->Object->New(ser::VarientType::Object);
                compObj->Add("Type", component->GetType()->GetName());
                compObj->Add("Id", component->GetObjectHandle().to_string());

                // serialize component
                {
                    auto comp = s->Object->New(ser::VarientType::Object);
                    ComponentSerializer componentSerializer {comp, s->IsWrite, s->HasEditorData};
                    component->Serialize(&componentSerializer);
                    compObj->Add("Data", comp);
                }
                componentsObj->Push(compObj);
            }
            s->Object->Add("Components", componentsObj);
        }
        else
        {
            SetName(s->Object->At("Name")->AsString());
            m_active = s->Object->At("IsActive")->AsBool();

            auto componentArr = s->Object->At("Components");
            for (int i = 0; i < componentArr->GetCount(); ++i)
            {
                auto componentObj = componentArr->At(i);
                // deserialize component description
                auto typeStr = componentObj->At("Type")->AsString();
                auto idStr = componentObj->At("Id")->AsString();
                auto componentType = AssemblyManager::GlobalFindType(typeStr);
                auto id = ObjectHandle::parse(idStr);

                // deserialize component
                auto compData = componentObj->At("Data");
                if (componentType)
                {
                    auto newComponent = ConstructComponent(componentType, id);
                    ComponentSerializer ser {compData, s->IsWrite, s->HasEditorData};
                    newComponent->Serialize(&ser);
                }
            }
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
                node = parent->GetNode();
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
            if (child->GetNode()->GetIsActiveSelf())
            {
                child->GetNode()->OnParentActiveChanged();
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
            return p->GetNode();
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
        m_nodeId = GetRuntimeWorld()->AllocElementId(GetObjectHandle());
    }
    void Node::OnInactive()
    {
        for (auto& comp : *m_components)
        {
            EndComponent(comp);
        }
        GetRuntimeWorld()->FreeElementId(m_nodeId);
        m_nodeId = 0;
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

    void Node::OnTransformChanged()
    {
        for (auto& comp : *this->m_components)
        {
            comp->OnTransformChanged();
        }
    }
    TransformComponent* Node::GetTransform() const
    {
        return m_transform;
    }
    BoxSphereBounds3f Node::GetBounds()
    {
        BoxSphereBounds3f bounds{};
        for (auto& comp : *m_components)
        {
            if (comp->HasBounds())
            {
                bounds = bounds + comp->GetBoundsWS();
            }

        }
        return bounds;
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
            DestroyObject(child->GetNode());
        }

        for (auto& comp : *m_components)
        {
            DestroyObject(comp, true);
        }
    }

    void Node::BeginNode(ObjectPtr<Scene> scene)
    {
        m_runtimeScene = std::move(scene);
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

    void Node::BeginPlay()
    {
        auto comps = *this->m_components;
        for (auto& comp : comps)
        {
            if (comp)
            {
                comp->BeginPlay();
            }
        }
    }

    void Node::EndPlay()
    {
        auto comps = *this->m_components;
        for (auto& comp : comps)
        {
            if (comp)
            {
                comp->EndPlay();
            }
        }
    }

    ObjectPtr<Component> Node::ConstructComponent(Type* type, const ObjectHandle& handle)
    {
        Object_sp obj = type->CreateSharedInstance({});
        Component_sp component = sptr_cast<Component>(obj);
        component->Construct(handle);

        // init
        component->m_ownerNode = self_ref();
        component->m_ownerNodePtr = this;

        bool isTransform = type->IsSubclassOf(cltypeof<TransformComponent>());
        if (!m_components->empty() && isTransform)
        {
            DestroyObject(m_components->at(0), true);
            m_components->at(0) = component;
        }
        else
        {
            this->m_components->push_back(component);
        }
        if (isTransform)
        {
            m_transform = sptr_cast<TransformComponent>(component).get();
        }

        return component;
    }

    ObjectPtr<Component> Node::AddComponent(Type* type)
    {
        auto component = ConstructComponent(type, {});

        if (m_runtimeScene && m_runtimeScene->GetWorld())
        {
            BeginComponent(component);
            component->OnTransformChanged();
            if (GetRuntimeWorld()->GetPlaying())
            {
                component->BeginPlay();
            }
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
            if (GetRuntimeWorld()->GetPlaying())
            {
                component->EndPlay();
            }
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


    }

    void Node::OnTick(Ticker ticker)
    {
        for (auto& comp : *this->m_components)
        {
            if (comp)
            {
                comp->OnTick(ticker);
            }
        }
    }
    void Node::SetLayer(int32_t layer)
    {
        m_layer = layer;
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
        if (m_runtimeScene)
        {
            return m_runtimeScene->GetWorld();
        }
        return nullptr;
    }
} // namespace pulsar
