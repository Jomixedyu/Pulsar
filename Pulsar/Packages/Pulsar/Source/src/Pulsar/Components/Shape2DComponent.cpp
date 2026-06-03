#include "Components/Shape2DComponent.h"
#include "Node.h"
#include "Physics2D/RigidBodyDynamics2DComponent.h"

namespace pulsar
{
    Shape2DComponent::Shape2DComponent()
    {
        m_canDrawGizmo = true;
    }

    void Shape2DComponent::SetIsSensor(bool v)
    {
        if (m_isSensor == v)
            return;
        m_isSensor = v;

        auto node = GetNode();
        while (node)
        {
            if (auto rb = node->GetComponent<RigidBodyDynamics2DComponent>())
            {
                rb->RequestRebuild();
                break;
            }
            node = node->GetParent();
        }
    }
} // namespace pulsar
