#include <Pulsar/Scene.h>
#include <Pulsar/Node.h>
#include <CoreLib.Serialization/DataSerializer.h>

namespace pulsar
{
    static void _InitializeNode(Node_ref node)
    {
        for (auto& com : node->GetAllComponentArray())
        {
            com->OnInitialize();
        } 

        for (auto& child : node->GetChildrenArray())
        {
            _InitializeNode(child);
        } 
    }
    void Scene::AddNode(Node_ref node)
    {
        assert(IsValid(node));

        this->m_sceneNodes->push_back(node);
        _InitializeNode(node);
    }
    void Scene::RemoveNode(Node_ref node)
    {
        auto it = std::find(this->m_sceneNodes->begin(), this->m_sceneNodes->end(), node);
        this->m_sceneNodes->erase(it);
    }

    Scene::Scene()
    {
        this->m_sceneNodes = mksptr(new List<Node_ref>);
    }
    
    ObjectPtr<Scene> Scene::StaticCreate(string_view name)
    {
        auto self = mksptr(new Scene);
        self->Construct();
        self->name_ = name;

        return self;
    }
    void Scene::OnDestroy()
    {
        base::OnDestroy();
        for (auto& node : *m_sceneNodes)
        {
            DestroyObject(node);
        }
    }
}

