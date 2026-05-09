#include "Gizmos.h"

#include "Components/Component.h"
#include "Rendering/LineRenderObject.h"
#include "Rendering/GizmoIconBatchRenderObject.h"
#include "Assets/StaticMesh.h"
#include "AssetManager.h"
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

    void GizmoPainter::DrawTexture(const Vector3f& worldPos, float size, const RCPtr<Material>& material, const Color4f& tint)
    {
        GizmoIconRequest req{};
        req.WorldPos = worldPos;
        req.Size = size;
        req.Tint = tint;
        req.Material = material;
        Context.IconRequests.push_back(std::move(req));
    }

    GizmosManager::GizmosManager()
    {
    }
    GizmosManager::GizmosManager(World* world)
        : m_world(world)
    {
    }
    GizmosManager::~GizmosManager()
    {
    }

    void GizmosManager::Draw()
    {
        GizmoPainter gizmoPainter{};
        array_list<GizmoContext> ctxs;

        size_t totalPoint = 0;
        array_list<StaticMeshVertex> linePoints;
        array_list<GizmoIconBatchRenderObject::IconItem> iconItems;

        for (auto& comp : m_gizmoComponents)
        {
            gizmoPainter.Context = {};

            bool isSelected = m_world->IsSelectedNode(comp->GetNode());
            comp->OnDrawGizmo(&gizmoPainter, isSelected);

            ctxs.push_back(gizmoPainter.Context);
            totalPoint += gizmoPainter.Context.LinePoints.size();

            for (auto& req : gizmoPainter.Context.IconRequests)
            {
                Matrix4f mat{0};
                mat[0][0] = req.Size;
                mat[1][1] = req.Size;
                mat[2][2] = req.Size;
                mat[3][0] = req.WorldPos.x;
                mat[3][1] = req.WorldPos.y;
                mat[3][2] = req.WorldPos.z;
                mat[3][3] = 1.0f;
                iconItems.push_back(GizmoIconBatchRenderObject::IconItem{mat, req.Material});
            }
        }

        linePoints.reserve(totalPoint);

        for (auto& ctx : ctxs)
        {
            linePoints.append_range(ctx.LinePoints);
        }

        // Lines
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

        // Icons
        if (!iconItems.empty())
        {
            if (!m_iconBatchRenderObject)
            {
                m_iconBatchRenderObject = mksptr(new GizmoIconBatchRenderObject);
                m_iconBatchRenderObject->SetMesh(AssetManager::Get()->LoadAsset<StaticMesh>("Engine/Shapes/Plane"));
                m_world->AddRenderObject(m_iconBatchRenderObject);
            }
            m_iconBatchRenderObject->SetItems(iconItems);
        }
        else
        {
            if (m_iconBatchRenderObject)
            {
                m_world->RemoveRenderObject(m_iconBatchRenderObject);
                m_iconBatchRenderObject.reset();
            }
        }
    }
} // namespace pulsar