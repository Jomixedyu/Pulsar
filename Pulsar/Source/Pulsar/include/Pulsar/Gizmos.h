#pragma once
#include "EngineMath.h"
#include "ObjectBase.h"
#include "Rendering/LineRenderObject.h"
#include "Rendering/PrimitiveStruct.h"
#include "Rendering/RenderObject.h"

namespace pulsar
{
    enum class GizmosDrawType
    {
        Lines,
        Points,
        Image
    };

    struct GizmoContext
    {
        string Category;

        Color4f LineTint{};
        Matrix4f LineModelMatrix{};
        array_list<StaticMeshVertex> LinePoints;
        int LineWidth = 1;

        void SetPointsColor(Color4f color);
    };

    class GizmoPainter
    {
    public:
        void DrawLineArray(const array_list<StaticMeshVertex>& points);
        void DrawLines(const StaticMeshVertex* points, size_t count);
        void DrawLine(const StaticMeshVertex& a, const StaticMeshVertex& b);

        GizmoContext Context;

        inline static constexpr Color4f DefaultSelectedLineColor {0.3f, 1.f, 0.3f} ;
        inline static constexpr Color4f DefaultLineColor {0, 0.1f, 0.4f};
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


        bool GetEnabled() const { return m_enabledDraw; }
        void SetEnabled(bool value);
        void Draw();
        void OnEndDraw();

    private:
        SPtr<LineRenderObject> m_lineRenderObject;
        class World* m_world = nullptr;
        bool m_enabledDraw;
    };
}