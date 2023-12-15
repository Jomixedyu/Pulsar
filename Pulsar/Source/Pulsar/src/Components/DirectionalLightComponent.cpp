#include "Components/DirectionalLightComponent.h"

#include "Node.h"
#include "TransformUtil.h"
#include "World.h"
#include <Pulsar/Rendering/LineRenderObject.h>

namespace pulsar
{

    void DirectionalLightComponent::BeginComponent()
    {
        base::BeginComponent();

        auto ro = mksptr(new LineRenderObject);
        Color4f lineColor {1,1,1};

        array_list<Vector3f> prototype;
        prototype.emplace_back(0,0,0);
        prototype.emplace_back(0,0,1);
        prototype.emplace_back(0,0.1,1);
        prototype.emplace_back(0,-0.1,1);
        prototype.emplace_back(0,-0.1,1);
        prototype.emplace_back(0,0,1.4);
        prototype.emplace_back(0,0,1.4);
        prototype.emplace_back(0,0.1,1);

        array_list<Vector3f> points;
        array_list<Color4f> colors;

        int num = 3;
        for (int i = 0; i < num; ++i)
        {
            auto mat = transutil::Rotate(Quat4f::FromEuler({0,0,360.f / num * i })) * transutil::Translate(Vector3f{0.f,0.2f,0.f});
            for (auto& pos : prototype)
            {
                points.push_back(mat * pos);
                colors.push_back(lineColor);
            }
        }
        ro->SetPoints(points, colors);


        m_gizmos = ro;
        GetWorld()->AddRenderObject(m_gizmos);
    }
    void DirectionalLightComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->RemoveRenderObject(m_gizmos);
        m_gizmos.reset();
    }
    void DirectionalLightComponent::OnMsg_TransformChanged()
    {
        base::OnMsg_TransformChanged();
        m_gizmos->SetTransform(GetAttachedNode()->GetTransform()->GetLocalToWorldMatrix());
    }

} // namespace pulsar
