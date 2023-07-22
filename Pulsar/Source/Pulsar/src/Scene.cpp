#include <Pulsar/Scene.h>
#include <Pulsar/Node.h>
#include <CoreLib.Serialization/DataSerializer.h>

namespace pulsar
{
    static void _InitializeNode(Node_sp node)
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
    void Scene::AddNode(Node_rsp node)
    {
        assert(node->IsAlive());
        this->scene_nodes_->push_back(node);
        _InitializeNode(node);
    }
    void Scene::RemoveNode(Node_rsp node)
    {
        auto it = std::find(this->scene_nodes_->begin(), this->scene_nodes_->end(), node);
        this->scene_nodes_->erase(it);
    }

    void Scene::SerializeBuildData(ser::Stream& stream, bool is_ser)
    {
        int node_len = this->scene_nodes_->size();
        ser::ReadWriteStream(stream, is_ser, node_len);

        for (size_t i = 0; i < node_len; i++)
        {

        }
    }

    Scene::Scene()
    {
        this->scene_nodes_ = mksptr(new List<Node_sp>);
    }
    
    sptr<Scene> Scene::StaticCreate(string_view name)
    {
        auto self = mksptr(new Scene);
        self->Construct();
        self->name_ = name;

        return self;
    }
}

