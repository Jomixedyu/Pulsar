#include "Gizmos.h"

#include "Components/Component.h"
#include "Rendering/LineRenderObject.h"
#include "World.h"

#include <memory>

namespace pulsar
{


    void GizmoPainter::DrawLineArray(const array_list<StaticMeshVertex>& points)
    {
        Context.LinePoints.append_range(points);
    }

    void GizmoPainter::DrawLines(const StaticMeshVertex* points, size_t count)
    {
        for (int i = 0; i < count; ++i)
        {
            Context.LinePoints.push_back(points[i]);
        }
    }
    void GizmoPainter::DrawLine(const StaticMeshVertex& a, const StaticMeshVertex& b)
    {
        Context.LinePoints.push_back(a);
        Context.LinePoints.push_back(b);
    }

    GizmosManager::GizmosManager(World* world)
        : m_world(world)
    {
    }
    GizmosManager::~GizmosManager()
    {
    }

    void GizmosManager::SetEnabled(bool value)
    {
        if (m_enabledDraw == value)
        {
            return;
        }
        m_enabledDraw = value;
        if (!value)
        {
            OnEndDraw();
        }
    }

    void GizmosManager::Draw()
    {
        if (!m_enabledDraw)
        {
            return;
        }
        GizmoPainter gizmoPainter{};
        array_list<GizmoContext> ctxs;

        size_t totalPoint = 0;
        array_list<StaticMeshVertex> linePoints;

        for (auto& comp : m_gizmoComponents)
        {
            gizmoPainter.Context = {};

            bool isSelected = m_world->IsSelectedNode(comp->GetNode());
            comp->OnDrawGizmo(&gizmoPainter, isSelected);

            ctxs.push_back(gizmoPainter.Context);
            totalPoint += gizmoPainter.Context.LinePoints.size();
        }

        linePoints.reserve(totalPoint);

        for (auto& ctx : ctxs)
        {
            linePoints.append_range(ctx.LinePoints);
        }

        if (!linePoints.empty())
        {
            if (m_lineRenderObject == nullptr)
            {
                m_lineRenderObject = mksptr(new LineRenderObject);
            }
            m_lineRenderObject->SetVerties(linePoints);

            m_world->AddRenderObject(m_lineRenderObject);
            m_lineRenderObject->SetTransform(Matrix4f{1});
        }
        else
        {
            if (m_lineRenderObject)
            {
                m_world->RemoveRenderObject(m_lineRenderObject);
                m_lineRenderObject.reset();
            }
        }
    }

    void GizmosManager::OnEndDraw()
    {
        if (m_lineRenderObject)
        {
            m_world->RemoveRenderObject(m_lineRenderObject);
            m_lineRenderObject.reset();
        }
    }

} // namespace pulsar