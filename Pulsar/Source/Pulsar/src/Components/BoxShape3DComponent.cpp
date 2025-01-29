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

    void BoxShape3DComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        static auto array = IdentityBox<Vector3f>();

        StaticMeshVertex vertices[24];

        auto local2World = GetTransform()->GetLocalToWorldMatrix();
        auto color = selected ? GizmoPainter::DefaultSelectedLineColor : GizmoPainter::DefaultLineColor;

        for (int i = 0; i < 24; ++i)
        {
            vertices[i].Position = local2World * array[i];
            vertices[i].Color = color;
        }

        if (selected)
        {
            painter->DrawLines(vertices, 24);
        }
    }

} // namespace pulsar