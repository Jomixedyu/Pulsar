#include "Components/PointLightComponent.h"

#include "Node.h"
#include "Rendering/LineRenderObject.h"
#include "Rendering/LightProxy.h"
#include "Rendering/PrimitiveStruct.h"
#include "Application.h"
#include "Rendering/RenderThread.h"
#include "World.h"

namespace pulsar
{



    static void CreateCircleLines(array_list<StaticMeshVertex>& verts, int segmentCount, float radius, Color4b color)
    {
        auto theta = jmath::Radians(360.f / (float)segmentCount);
        for (int i = 0; i < segmentCount; ++i)
        {
            auto& p1 = verts.emplace_back();
            p1.Position = Vector3f{cos(theta * i), 0, sin(theta * i)} * radius;
            p1.Color = color;
            auto& p2 = verts.emplace_back();
            p2.Position = Vector3f{cos(theta * float(i + 1)), 0, sin(theta * float(i + 1))} * radius;
            p2.Color = color;
        }
    }

    static void CreateSphereVolumeLines(array_list<StaticMeshVertex>& verts, int segmentCount, float radius, const Matrix4f& transform, Color4b defaultColor)
    {
        array_list<StaticMeshVertex> circle;
        circle.reserve(segmentCount * 2);
        CreateCircleLines(circle, segmentCount, radius, defaultColor);

        auto xformZ = jmath::Rotate(Quat4f::FromEuler({0, 0, 90}));
        auto xformX = jmath::Rotate(Quat4f::FromEuler({90, 0, 0}));

        for (const auto& vert : circle)
        {
            auto newVert = vert;
            newVert.Position = transform * newVert.Position;
            verts.push_back(newVert);
        }
        for (const auto& vert : circle)
        {
            auto newVert = vert;
            newVert.Position = transform * (xformZ * newVert.Position);
            verts.push_back(newVert);
        }
        for (const auto& vert : circle)
        {
            auto newVert = vert;
            newVert.Position = transform * (xformX * newVert.Position);
            verts.push_back(newVert);
        }
    }

    void PointLightComponent::BeginComponent()
    {
        m_canDrawGizmo = true;
        base::BeginComponent();

        OnTransformChanged();
        OnLightColorChanged();
        OnRadiusChanged();
    }

    void PointLightComponent::EndComponent()
    {
        base::EndComponent();
    }

    SPtr<rendering::RenderProxy> PointLightComponent::CreateRenderProxy()
    {
        return mksptr(new PointLightProxy());
    }

    void PointLightComponent::SyncRenderProxy()
    {
        auto proxy = std::static_pointer_cast<PointLightProxy>(m_proxy);
        if (!proxy)
            return;
        LightShaderParameter param = m_runtimeLightData;
        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [proxy, param](gfx::GFXResourceManager*) mutable
            {
                proxy->Param = param;
            });
    }

    BoxSphereBounds3f PointLightComponent::GetBoundsWS()
    {
        return BoxSphereBounds3f{ SphereBounds3f(GetTransform()->GetWorldPosition(), m_radius) };
    }

    void PointLightComponent::SetRadius(float value)
    {
        m_radius = value;
        OnRadiusChanged();
    }

    void PointLightComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        auto lineColor = selected ? painter->DefaultSelectedLineColor : painter->DefaultLineColor;

        static array_list<StaticMeshVertex> verts;
        verts.clear();

        CreateSphereVolumeLines(verts, 36, m_radius, math::Translate(GetTransform()->GetWorldPosition()), lineColor);
        painter->DrawLineArray(verts);
    }

    void PointLightComponent::OnTransformChanged()
    {
        base::OnTransformChanged();
        m_runtimeLightData.WorldPosition = GetTransform()->GetWorldPosition();
        MarkRenderStateDirty();
    }

    void PointLightComponent::OnRadiusChanged()
    {
        base::OnLightColorChanged();
        m_runtimeLightData.SourceAndSoftSourceRadius.x = m_radius;
        MarkRenderStateDirty();
    }

    void PointLightComponent::OnLightColorChanged()
    {
        base::OnLightColorChanged();
        m_runtimeLightData.Color = Vector4f(m_lightColor.r, m_lightColor.g, m_lightColor.b, m_intensity);
        MarkRenderStateDirty();
    }
    void PointLightComponent::OnIntensityChanged()
    {
        base::OnIntensityChanged();
        m_runtimeLightData.Color = Vector4f(m_lightColor.r, m_lightColor.g, m_lightColor.b, m_intensity);
        MarkRenderStateDirty();
    }

    void PointLightComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == NAMEOF(m_radius))
        {
            OnRadiusChanged();
        }

    }
} // namespace pulsar
