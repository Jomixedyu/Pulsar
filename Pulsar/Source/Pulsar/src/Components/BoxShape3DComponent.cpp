#include "Components/BoxShape3DComponent.h"

#include "Components/TransformComponent.h"

namespace pulsar
{
    template<typename _Vector>
    static std::array<_Vector, 24> IdentityBox()
    {
        constexpr auto x = 0.5f;
        constexpr auto y = 0.5f;
        constexpr auto z = 0.5f;

        auto a = _Vector{ -x,  y, -z };
        auto b = _Vector{ -x,  y,  z };
        auto c = _Vector{  x,  y,  z };
        auto d = _Vector{  x,  y, -z };
        auto e = _Vector{ -x, -y, -z };
        auto f = _Vector{ -x, -y,  z };
        auto g = _Vector{  x, -y,  z };
        auto h = _Vector{  x, -y, -z };

        return {
            a,b, b,c, c,d, d,a,
            e,f, f,g, g,h, h,e,
            a,e, b,f, c,g, d,h
        };
    }

    static void DrawBoxLine(Vector3f pos, Vector3f extent, StaticMeshVertex vertices[12])
    {
        auto a = pos + Vector3f(-extent.x, extent.y, -extent.z);
    }

    void BoxShape3DComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        StaticMeshVertex vertices[12];
        if (selected)
        {
            auto pos = GetTransform()->GetLocalToWorldMatrix();
            DrawBoxLine(pos, m_halfSize, vertices);
            painter->DrawLines(vertices, 12);
        }
    }

} // namespace pulsar