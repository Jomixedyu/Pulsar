#include "Components/PointLightComponent.h"

#include "Node.h"
#include "Rendering/LineRenderObject.h"
#include "Rendering/PrimitiveStruct.h"
#include "World.h"

namespace pulsar
{
    array_list<StaticMeshVertex> CreateSphereVolume(int segmentCount, Color4f defaultColor)
    {
        array_list<StaticMeshVertex> verts;
        auto theta = jmath::Radians(360.f / (float)segmentCount);
        for (int i = 0; i < segmentCount; ++i)
        {
            auto& p1 = verts.emplace_back();
            p1.Position = Vector3f{cos(theta * i), 0, sin(theta * i)};
            p1.Color = defaultColor;
            auto& p2 = verts.emplace_back();
            p2.Position = Vector3f{cos(theta * float(i + 1)), 0, sin(theta * float(i + 1))};
            p2.Color = defaultColor;
        }
        {
            auto xform = jmath::Rotate(Quat4f::FromEuler({0, 0, 90}));
            auto copy = verts;
            for (auto& vert : copy)
            {
                vert.Position = xform * vert.Position;
            }
            verts.append_range(std::move(copy));
        }
        {
            auto xform = jmath::Rotate(Quat4f::FromEuler({90, 0, 0}));
            auto copy = verts;
            for (auto& vert : copy)
            {
                vert.Position = xform * vert.Position;
            }
            verts.append_range(std::move(copy));
        }

        return verts;
    }

    void PointLightComponent::BeginComponent()
    {
        base::BeginComponent();
        auto ro = mksptr(new LineRenderObject);
        m_gizmos = ro;
        auto sphere = CreateSphereVolume(36, {1, 1, 1, 1});
        ro->SetVerties(sphere);
        GetWorld()->AddRenderObject(ro);
    }
    void PointLightComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->RemoveRenderObject(m_gizmos);
        m_gizmos.reset();
        m_gizmos.reset();
    }
    void PointLightComponent::OnMsg_TransformChanged()
    {
        base::OnMsg_TransformChanged();
        m_gizmos->SetTransform(GetAttachedNode()->GetTransform()->GetLocalToWorldMatrix());
    }
} // namespace pulsar
