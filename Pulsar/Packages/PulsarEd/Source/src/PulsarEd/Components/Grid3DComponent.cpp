#include "Components/Grid3DComponent.h"

#include "Pulsar/Components/StaticMeshRendererComponent.h"

#include "Pulsar/Application.h"
#include "Pulsar/AssetManager.h"
#include "Pulsar/Assets/StaticMesh.h"
#include "Pulsar/Rendering/LineRenderObject.h"
#include "Pulsar/Rendering/RenderThread.h"
#include "PulsarEd/Editors/CommonPanel/SceneWindow.h"
#include "PulsarEd/Windows/EditorWindowManager.h"
#include <gfx/GFXResourceManager.h>

namespace pulsared
{

    void Grid3DComponent::BeginComponent()
    {
        int line_count = 20;
        float detail_distance = 1;
        float total_width = detail_distance * line_count;

        Color4f detailLineColor = Color4f(0.2f, 0.2f, 0.2f, 1);

        for (int x = -line_count / 2; x <= line_count / 2; x++)
        {
            if (x == 0)
            {
                auto color = detailLineColor;
                color.r = 0.9f;
                m_vert.emplace_back(total_width / 2, 0, detail_distance * x);
                m_vert.emplace_back(0, 0, detail_distance * x);
                m_vert.emplace_back(0, 0, detail_distance * x);
                m_vert.emplace_back(-total_width / 2, 0, detail_distance * x);
                m_colors.push_back(MakeColor4b(color));
                m_colors.push_back(MakeColor4b(color));
                color.r = 0.4f;
                m_colors.push_back(MakeColor4b(color));
                m_colors.push_back(MakeColor4b(color));
            }
            else
            {
                m_vert.emplace_back(total_width / 2, 0, detail_distance * x);
                m_vert.emplace_back(-total_width / 2, 0, detail_distance * x);
                m_colors.push_back(MakeColor4b(detailLineColor));
                m_colors.push_back(MakeColor4b(detailLineColor));
            }
        }
        for (int z = -line_count / 2; z <= line_count / 2; z++)
        {
            if (z == 0)
            {
                auto color = detailLineColor;
                color.r = 0.15f;
                color.g = 0.18f;
                color.b = 1.f;
                m_vert.emplace_back(detail_distance * z, 0, total_width / 2);
                m_colors.push_back(MakeColor4b(color));
                m_vert.emplace_back(detail_distance * z, 0, 0);
                m_colors.push_back(MakeColor4b(color));
                color.b = 0.4f;
                m_vert.emplace_back(detail_distance * z, 0, 0);
                m_colors.push_back(MakeColor4b(color));
                m_vert.emplace_back(detail_distance * z, 0, -total_width / 2);
                m_colors.push_back(MakeColor4b(color));
            }
            else
            {
                m_vert.emplace_back(detail_distance * z, 0, total_width / 2);
                m_vert.emplace_back(detail_distance * z, 0, -total_width / 2);
                m_colors.push_back(MakeColor4b(detailLineColor));
                m_colors.push_back(MakeColor4b(detailLineColor));
            }
        }

        m_vert.emplace_back(0, 0, 0);
        m_vert.emplace_back(0, 1, 0);
        m_colors.push_back(jmath::MakeColor4b(0.f, 1, 0, 1));
        m_colors.push_back(jmath::MakeColor4b(0.f, 1, 0, 1));

        // Verts are ready: base::BeginComponent registers the proxy (CreateRenderProxy).
        base::BeginComponent();
        m_renderObject = sptr_static_cast<LineRenderObject>(m_proxy);
        OnTransformChanged();
    }
    void Grid3DComponent::EndComponent()
    {
        m_renderObject.reset();
        base::EndComponent();
    }
    void Grid3DComponent::OnTransformChanged()
    {
        base::OnTransformChanged();
        MarkRenderStateDirty();
    }
    void Grid3DComponent::ResolveRenderStateDirty()
    {
        if (!m_renderObject)
            return;
        Matrix4f localToWorld = GetNode()->GetTransform()->GetLocalToWorldMatrix();
        auto ro = m_renderObject;
        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [ro, localToWorld](gfx::GFXResourceManager*) mutable
            {
                ro->SetTransform(localToWorld);
            });
    }

    SPtr<rendering::RenderProxy> Grid3DComponent::CreateRenderProxy()
    {
        auto ro = new LineRenderObject();
        ro->SetDepthTestEnabled(true);
        ro->SetQueue(ShaderPassRenderQueueType::Opaque);
        ro->SetPoints(m_vert, m_colors);

        return mksptr(ro);
    }

} // namespace pulsared