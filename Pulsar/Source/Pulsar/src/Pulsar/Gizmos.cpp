#include "Gizmos.h"

#include "Components/Component.h"
#include "Rendering/LineRenderObject.h"
#include "Rendering/GizmoIconBatchRenderObject.h"
#include "Assets/StaticMesh.h"
#include "Assets/Shader.h"
#include "Assets/Material.h"
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

    void GizmoPainter::DrawTexture(const Vector3f& worldPos, float size, const RCPtr<Texture2D>& texture, const Color4f& tint)
    {
        GizmoIconRequest req{};
        req.WorldPos = worldPos;
        req.Size = size;
        req.Tint = tint;
        req.Texture = texture;
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
        array_list<GizmoIconRequest> iconRequests;

        for (auto& comp : m_gizmoComponents)
        {
            gizmoPainter.Context = {};

            bool isSelected = m_world->IsSelectedNode(comp->GetNode());
            comp->OnDrawGizmo(&gizmoPainter, isSelected);

            ctxs.push_back(gizmoPainter.Context);
            totalPoint += gizmoPainter.Context.LinePoints.size();

            for (auto& req : gizmoPainter.Context.IconRequests)
            {
                iconRequests.push_back(req);
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
        if (!iconRequests.empty())
        {
            if (!m_iconBatchRenderObject)
            {
                m_iconBatchRenderObject = mksptr(new GizmoIconBatchRenderObject);
                m_iconBatchRenderObject->SetMesh(AssetManager::Get()->LoadAsset<StaticMesh>("Engine/Shapes/Plane"));
                m_world->AddRenderObject(m_iconBatchRenderObject);
            }

            // Ensure billboard shader ready
            if (!m_billboardShader)
            {
                m_billboardShader = AssetManager::Get()->LoadAsset<Shader>("Engine/Shaders/UnlitBillboard");
            }

            // Expand material pool if needed
            while (m_iconMaterialPool.size() < iconRequests.size())
            {
                auto mat = Material::StaticCreate(m_billboardShader);
                mat->CreateGPUResource();
                m_iconMaterialPool.push_back(mat);
            }

            // Assign texture/tint per icon
            array_list<GizmoIconBatchRenderObject::IconItem> batchItems;
            batchItems.reserve(iconRequests.size());
            for (size_t i = 0; i < iconRequests.size(); ++i)
            {
                auto& req = iconRequests[i];
                auto& mat = m_iconMaterialPool[i];
                if (req.Texture)
                {
                    mat->SetTexture("_BaseColorMap", req.Texture);
                }
                mat->SetVector4("_TintColor", Vector4f{req.Tint.r, req.Tint.g, req.Tint.b, req.Tint.a});
                mat->SubmitParameters();

                Matrix4f matx{0};
                matx[0][0] = req.Size;
                matx[1][1] = req.Size;
                matx[2][2] = req.Size;
                matx[3][0] = req.WorldPos.x;
                matx[3][1] = req.WorldPos.y;
                matx[3][2] = req.WorldPos.z;
                matx[3][3] = 1.0f;
                batchItems.push_back(GizmoIconBatchRenderObject::IconItem{matx, mat});
            }

            m_iconBatchRenderObject->SetItems(batchItems);
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