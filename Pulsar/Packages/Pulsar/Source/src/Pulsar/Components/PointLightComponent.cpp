#include "Components/PointLightComponent.h"

#include "Node.h"
#include "Rendering/LineRenderObject.h"
#include "Rendering/LightProxy.h"
#include "Rendering/PrimitiveStruct.h"
#include "Application.h"
#include "Rendering/RenderThread.h"
#include "TransformUtil.h"
#include "World.h"

namespace pulsar
{


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

    void PointLightComponent::ResolveRenderStateDirty()
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

        Matrix4f matrix{1};
        transutil::NewScale(matrix, Vector3f{m_radius, m_radius, m_radius});
        transutil::NewTranslate(matrix, GetTransform()->GetWorldPosition());
        painter->DrawWireSphere(matrix, lineColor);
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
