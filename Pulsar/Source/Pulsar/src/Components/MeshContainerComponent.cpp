#include "Components/MeshContainerComponent.h"
#include "Node.h"

namespace pulsar
{
    MessageId MeshContainerComponent::MSG_MeshUpdate()
    {
        static bool b;
        return reinterpret_cast<MessageId>(&b);
    }

    StaticMesh_sp MeshContainerComponent::GetMesh() const
    {
        return m_mesh;
    }
    void MeshContainerComponent::SetMesh(StaticMesh_sp value)
    {
        m_mesh = value;
        this->GetAttachedNode()->SendMessage(MSG_MeshUpdate());
    }
}
