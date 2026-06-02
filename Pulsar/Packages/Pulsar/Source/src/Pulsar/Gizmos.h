#pragma once
#include "EngineMath.h"
#include "ObjectBase.h"
#include "Rendering/LineRenderObject.h"
#include "Rendering/PrimitiveStruct.h"
#include "Rendering/RenderObject.h"
#include <Pulsar/Assets/Material.h>

namespace pulsar
{
    enum class GizmosDrawType
    {
        Lines,
        Points,
        Image
    };

    struct GizmoIconRequest
    {
        Vector3f WorldPos;
        float Size = 1.0f;
        Color4f Tint = Color4f{1, 1, 1, 1};
        RCPtr<class Texture2D> Texture;
    };

    struct GizmoContext
    {
        string Category;

        Color4f LineTint{};
        Matrix4f LineModelMatrix{};
        array_list<StaticMeshVertex> LinePoints;
        int LineWidth = 1;

        array_list<GizmoIconRequest> IconRequests;

        void SetPointsColor(Color4f color);
    };

    class GizmoPainter
    {
    public:
        void DrawLineArray(const array_list<StaticMeshVertex>& points);
        void DrawLines(const StaticMeshVertex* points, size_t count);
        void DrawLine(const StaticMeshVertex& a, const StaticMeshVertex& b);

        void DrawTexture(const Vector3f& worldPos, float size, const RCPtr<class Texture2D>& texture, const Color4f& tint = Color4f{1, 1, 1, 1});

        GizmoContext Context;

        inline static constexpr Color4b DefaultSelectedLineColor {uint8_t(255 * 0.3f), uint8_t(255 * 1.f), uint8_t(255 * 0.3f) } ;
        inline static constexpr Color4b DefaultLineColor {0, uint8_t(255 * 0.1f), uint8_t(255 * 0.4f) };
    };

    class Component;
    class GizmosManager final
    {
    public:
        GizmosManager();
        GizmosManager(const GizmosManager&) = delete;
        GizmosManager(GizmosManager&&) = delete;
        explicit GizmosManager(class World* world);
        ~GizmosManager();
    private:
        array_list<ObjectPtr<Component>> m_gizmoComponents;
    public:
        void AddGizmoComponent(const ObjectPtr<Component>& component)
        {
            m_gizmoComponents.push_back(component);
        }
        void RemoveGizmoComponent(const ObjectPtr<Component>& component)
        {
            std::erase(m_gizmoComponents, component);
        }


        void Draw();

    private:
        SPtr<LineRenderObject> m_lineRenderObject;
        SPtr<class GizmoIconBatchRenderObject> m_iconBatchRenderObject;
        RCPtr<class Shader> m_billboardShader;
        array_list<RCPtr<class Material>> m_iconMaterialPool;
        class World* m_world = nullptr;
    };
}