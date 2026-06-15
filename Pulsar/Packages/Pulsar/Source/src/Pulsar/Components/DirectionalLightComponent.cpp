#include "Components/DirectionalLightComponent.h"

#include "Node.h"
#include "Scene.h"
#include "TransformUtil.h"
#include "World.h"
#include "Application.h"
#include <Pulsar/Rendering/LineRenderObject.h>
#include <Pulsar/Rendering/LightProxy.h>
#include <Pulsar/Rendering/RenderThread.h>

namespace pulsar
{

    void DirectionalLightComponent::BeginComponent()
    {
        base::BeginComponent();

        auto ro = mksptr(new LineRenderObject);
        Color4b lineColor {255,255,255};

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
        array_list<Color4b> colors;

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

        OnTransformChanged();
        OnIntensityChanged();
        OnLightColorChanged();
    }

    void DirectionalLightComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->RemoveRenderObject(m_gizmos);
        m_gizmos.reset();
    }

    SPtr<rendering::RenderProxy> DirectionalLightComponent::CreateRenderProxy()
    {
        return mksptr(new DirectionalLightProxy());
    }

    void DirectionalLightComponent::SyncRenderProxy()
    {
        auto proxy = std::static_pointer_cast<DirectionalLightProxy>(m_proxy);
        if (!proxy)
            return;
        Vector3f vector = m_vector;
        Color4f  color = m_lightColor;
        float    intensity = m_intensity;
        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [proxy, vector, color, intensity](gfx::GFXResourceManager*) mutable
            {
                proxy->Vector = vector;
                proxy->Color = color;
                proxy->Intensity = intensity;
            });
    }

    void DirectionalLightComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
    }

    void DirectionalLightComponent::OnIntensityChanged()
    {
        base::OnIntensityChanged();
        MarkRenderStateDirty();
    }

    void DirectionalLightComponent::OnLightColorChanged()
    {
        base::OnLightColorChanged();
        MarkRenderStateDirty();
    }

    void DirectionalLightComponent::OnTransformChanged()
    {
        base::OnTransformChanged();

        m_vector = GetNode()->GetTransform()->GetForward();
        m_gizmos->SetTransform(GetNode()->GetTransform()->GetLocalToWorldMatrix());
        MarkRenderStateDirty();
    }

} // namespace pulsar
