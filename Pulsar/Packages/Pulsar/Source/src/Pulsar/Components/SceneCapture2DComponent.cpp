#include "Components/SceneCapture2DComponent.h"

#include "Application.h"
#include "Node.h"
#include "AssetManager.h"
#include "Assets/StaticMesh.h"
#include "Rendering/SceneViewProxy.h"
#include <Pulsar/Rendering/RenderThread.h>
#include <Pulsar/Rendering/RenderTextureProxy.h>
#include <gfx/GFXResourceRegistry.h>

namespace pulsar
{
    SceneCapture2DComponent::SceneCapture2DComponent()
        : m_debugViewMat({})
    {
        m_renderingPath = RenderingPathMode::Deferred;
        m_projectionMode = CaptureProjectionMode::Perspective;
        m_fov = 60.0f;
        m_near = 0.3f;
        m_far = 1000.0f;
        m_backgroundColor = Color4f{0.1f, 0.1f, 0.1f, 1.0f};
        m_canDrawGizmo = true;
        m_gizmoTexture = AssetManager::Get()->LoadAsset<Texture2D>("PulsarEd/Gizmos/cinema");
    }

    Matrix4f SceneCapture2DComponent::GetViewMat() const
    {
        return GetNode()->GetTransform()->GetWorldToLocalMatrix();
    }

    Matrix4f SceneCapture2DComponent::GetProjectionMat() const
    {
        const Vector2f& size = this->m_renderTarget->GetSize2df();
        Matrix4f ret{};
        if (this->m_projectionMode == CaptureProjectionMode::Perspective)
        {
            math::Perspective_LHZO(ret,
                                   math::Radians(this->m_fov),
                                   size.x / size.y,
                                   this->m_near,
                                   this->m_far);
        }
        else
        {
            math::Ortho_LHZO(ret,
                             -size.x / 2 * m_orthoSize / 100,
                             size.x / 2 * m_orthoSize / 100,
                             -size.y / 2 * m_orthoSize / 100,
                             size.y / 2 * m_orthoSize / 100,
                             this->m_near,
                             this->m_far);
        }
        return ret;
    }

    Matrix4f SceneCapture2DComponent::GetInvViewProjectionMat() const
    {
        return Inverse(GetProjectionMat() * GetViewMat());
    }

    void SceneCapture2DComponent::SetFOV(float value)
    {
        m_fov = value;
        UpdateCBuffer();
        m_renderDirtyCamera = true;
        MarkRenderStateDirty();
    }
    void SceneCapture2DComponent::SetNear(float value)
    {
        m_near = value;
        UpdateCBuffer();
        m_renderDirtyCamera = true;
        MarkRenderStateDirty();
    }
    void SceneCapture2DComponent::SetFar(float value)
    {
        m_far = value;
        UpdateCBuffer();
        m_renderDirtyCamera = true;
        MarkRenderStateDirty();
    }
    void SceneCapture2DComponent::SetBackgroundColor(const Color4f& value)
    {
        m_backgroundColor = value;
        UpdateRTBackgroundColor();
        UpdateCBuffer();
        m_renderDirtyCamera = true;
        MarkRenderStateDirty();
    }
    void SceneCapture2DComponent::SetProjectionMode(CaptureProjectionMode mode)
    {
        m_projectionMode = mode;
        UpdateCBuffer();
        m_renderDirtyCamera = true;
        MarkRenderStateDirty();
    }
    void SceneCapture2DComponent::SetOrthoSize(float value)
    {
        m_orthoSize = value;
        UpdateCBuffer();
        m_renderDirtyCamera = true;
        MarkRenderStateDirty();
    }

    void SceneCapture2DComponent::OnTransformChanged()
    {
        base::OnTransformChanged();
        UpdateCBuffer();
        m_renderDirtyTransform = true;
        m_renderDirtyPostProcess = true;
        MarkRenderStateDirty();
    }

    void SceneCapture2DComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);

        // Inspector edits write the reflected fields directly and bypass the
        // setters, so the render-state flags have to be raised here.
        const auto& name = info->GetName();
        if (name == NAMEOF(m_fov) || name == NAMEOF(m_near) || name == NAMEOF(m_far) ||
            name == NAMEOF(m_projectionMode) || name == NAMEOF(m_orthoSize))
        {
            m_renderDirtyCamera = true;
        }
        else if (name == NAMEOF(m_msaaSamples))
        {
            m_renderDirtyRenderTarget = true;
        }
        MarkRenderStateDirty();
    }

    bool SceneCapture2DComponent::ExtractViewData(SceneViewData& outData)
    {
        if (!m_renderTarget)
            return false;

        outData.ViewMatrix       = GetViewMat();
        outData.ProjectionMatrix = GetProjectionMat();
        outData.CameraPosition   = GetNode()->GetTransform()->GetWorldPosition();
        outData.CameraForward    = GetNode()->GetTransform()->GetForward();
        outData.Near             = m_near;
        outData.Far              = m_far;
        outData.Resolution       = m_renderTarget->GetSize2df();
        outData.BackgroundColor  = m_backgroundColor;
        outData.MSAASamples      = std::max(1, m_msaaSamples);
        outData.GizmoPassEnabled = false;
        outData.RenderTarget.Proxy       = m_renderTarget->GetRenderProxy();
        return true;
    }

    void SceneCapture2DComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        auto pos = GetNode()->GetTransform()->GetWorldPosition();
        painter->DrawTexture(pos, 0.5f, m_gizmoTexture);
    }


    void SceneCapture2DComponent::UpdateRTBackgroundColor()
    {
        if (!m_renderTarget)
        {
            return;
        }
        // The RT proxy initializes asynchronously on the render thread; route the
        // clear-color write through the same queue so it lands after Init (FIFO).
        auto proxy = m_renderTarget->GetRenderProxy();
        const auto color = m_backgroundColor;
        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [proxy, color](gfx::GFXResourceRegistry*)
            {
                auto view = proxy ? proxy->GetColorTextureView() : nullptr;
                if (auto* tex = view ? view->GetTexture() : nullptr)
                {
                    tex->SetTargetClearColor({ color.r, color.g, color.b, color.a });
                }
            });
    }

    void SceneCapture2DComponent::UpdateCBuffer()
    {
        // Any param/transform change re-snapshots the view proxy next frame.
        MarkRenderStateDirty();

        m_debugViewMat = GetViewMat();
    }
} // namespace pulsar
