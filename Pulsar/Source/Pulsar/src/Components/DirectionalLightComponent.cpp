#include "Components/DirectionalLightComponent.h"

#include "World.h"
#include <Pulsar/Rendering/LineRenderObject.h>

namespace pulsar
{

    void DirectionalLightComponent::BeginComponent()
    {
        base::BeginComponent();

        auto ro = mksptr(new LineRenderObject);

        ro->m_pointPairs.push_back({0,0,0});
        ro->m_pointPairs.push_back({0,0,1});
        ro->m_pointColors.push_back({1,1,1});
        ro->m_pointColors.push_back({1,1,1});

        m_gizmos = ro;
        GetWorld()->AddRenderObject(m_gizmos);
    }
    void DirectionalLightComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->RemoveRenderObject(m_gizmos);
        m_gizmos.reset();
    }

} // namespace pulsar
