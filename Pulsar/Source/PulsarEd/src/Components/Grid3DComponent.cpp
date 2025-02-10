#include "Components/Grid3DComponent.h"

#include "Pulsar/Components/StaticMeshRendererComponent.h"

#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/StaticMesh.h>
#include <Pulsar/Rendering/LineRenderObject.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <PulsarEd/Windows/EditorWindowManager.h>
#include <PulsarEd/Editors/CommonPanel/SceneWindow.h>

namespace pulsared
{

    void Grid3DComponent::BeginComponent()
    {
        base::BeginComponent();

        int line_count = 20;
        float detail_distance = 1;
        float total_width = detail_distance * line_count;

        Color4f detailLineColor = {0.2f, 0.2f, 0.2f, 1};

        for (int x = -line_count / 2; x <= line_count / 2; x++)
        {
            if (x == 0)
            {
                Color4f color = detailLineColor;
                color.r = 0.9f;
                m_vert.emplace_back(total_width / 2, 0, detail_distance * x);
                m_vert.emplace_back(0, 0, detail_distance * x);
                m_vert.emplace_back(0, 0, detail_distance * x);
                m_vert.emplace_back(-total_width / 2, 0, detail_distance * x);
                m_colors.push_back(color);
                m_colors.push_back(color);
                color.r = 0.4f;
                m_colors.push_back(color);
                m_colors.push_back(color);
            }
            else
            {
                m_vert.emplace_back(total_width / 2, 0, detail_distance * x);
                m_vert.emplace_back(-total_width / 2, 0, detail_distance * x);
                m_colors.push_back(detailLineColor);
                m_colors.push_back(detailLineColor);
            }
        }
        for (int z = -line_count / 2; z <= line_count / 2; z++)
        {
            if (z == 0)
            {
                Color4f color = detailLineColor;
                color.r = 0.15f;
                color.g = 0.18f;
                color.b = 1.f;
                m_vert.emplace_back(detail_distance * z, 0, total_width / 2);
                m_colors.push_back(color);
                m_vert.emplace_back(detail_distance * z, 0, 0);
                m_colors.push_back(color);
                color.b = 0.4f;
                m_vert.emplace_back(detail_distance * z, 0, 0);
                m_colors.push_back(color);
                m_vert.emplace_back(detail_distance * z, 0, -total_width / 2);
                m_colors.push_back(color);
            }
            else
            {
                m_vert.emplace_back(detail_distance * z, 0, total_width / 2);
                m_vert.emplace_back(detail_distance * z, 0, -total_width / 2);
                m_colors.push_back(detailLineColor);
                m_colors.push_back(detailLineColor);
            }
        }

        m_vert.emplace_back(0, 0, 0);
        m_vert.emplace_back(0, 1, 0);
        m_colors.push_back({0, 1, 0, 1});
        m_colors.push_back({0, 1, 0, 1});

        m_renderObject = CreateRenderObject();
        GetNode()->GetRuntimeWorld()->AddRenderObject(m_renderObject);
        OnTransformChanged();
    }
    void Grid3DComponent::EndComponent()
    {
        base::EndComponent();
        GetNode()->GetRuntimeWorld()->RemoveRenderObject(m_renderObject);
        m_renderObject.reset();
    }
    void Grid3DComponent::OnTransformChanged()
    {
        base::OnTransformChanged();
        m_renderObject->SetTransform(GetNode()->GetTransform()->GetLocalToWorldMatrix());
    }

    SPtr<rendering::RenderObject> Grid3DComponent::CreateRenderObject()
    {
        auto ro = new LineRenderObject();
        ro->SetPoints(m_vert, m_colors);

        return mksptr(ro);
    }

} // namespace pulsared