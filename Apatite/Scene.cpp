#include <Apatite/Scene.h>
#include <Apatite/Node.h>
#include <CoreLib.Serialization/DataSerializer.h>

namespace apatite
{
    void Scene::AddNode(Node_rsp node)
    {
        this->scene_nodes_->push_back(node);
    }
    void Scene::RemoveNode(Node_rsp node)
    {
        auto it = std::find(this->scene_nodes_->begin(), this->scene_nodes_->end(), node);
        this->scene_nodes_->erase(it);
    }

    void Scene::Serialize(ser::Stream& stream, bool is_ser)
    {
        int node_len = this->scene_nodes_->size();
        ser::ReadWriteStream(stream, is_ser, node_len);

        for (size_t i = 0; i < node_len; i++)
        {

        }
    }

    void Scene::OnUpdate()
    {
        for (auto& item : *this->scene_nodes_)
        {
            item->OnUpdate();
            //item->BroadcastSendMessage(MessageType::Update);
        }
    }
}

