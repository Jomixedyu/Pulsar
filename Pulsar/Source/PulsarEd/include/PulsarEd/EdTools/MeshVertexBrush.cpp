#include "MeshVertexBrush.h"

#include "EditorWorld.h"
#include "ImGuiExt.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"
#include "imgui/imgui.h"


namespace pulsared
{
    static bool LineSphereIntersection(const Vector3f& Start, const Vector3f& Dir, float length, const Vector3f& Origin, float Radius)
    {
        const auto	EO = Start - Origin;
        const auto	v =  Dot(Dir, (Origin - Start));
        const auto	disc = Radius * Radius - (Dot(EO, EO) - v * v);

        if(disc >= 0)
        {
            const auto Time = (v - std::sqrt(disc)) / length;

            if(Time >= 0 && Time <= 1)
                return true;
        }
        return false;
    }

    static float ComputeSquaredDistanceFromBoxToPoint(const BoxBounds3f& box, const Vector3f& point)
    {
        auto distance = Max(Abs(point - box.GetCenter()) - box.GetExtent(), Vector3f{});
        return Dot(distance, distance);
    }

    static bool RayIntersectTri(const Ray& ray, float length, const Triangle3f& p)
    {

    }

    static bool RayIntersect(const Vector3f& start, const Vector3f& end, Vector3i& hitTri, Vector3f& hitPos)
    {

        return false;
    }

    bool MeshVertexBrush::HitTest(const Ray& ray, HitResult& result, const RaycastFilter& filter)
    {
        auto selected = GetWorld()->GetSelection().GetSelected();
        if (!selected)
        {
            return false;
        }

        array_list<ObjectPtr<MeshRendererComponent>> components;
        selected->GetComponentsInChildren(components);

        if (components.empty())
        {
            return false;
        }

        const auto traceStart = ray.Origin;
        const auto traceEnd = ray.Origin + ray.Direction * 10000.f;

        const auto lengthVec = traceEnd - traceStart;
        const auto lengthSqr = Dot(lengthVec, lengthVec);

        array_list<HitResult> Results;
        for (const auto& component : components)
        {
            const auto compBounds = component->GetBoundsWS();
            const auto boxBounds = compBounds.GetBox();
            const auto sqrRadius = jmath::Square(compBounds.Radius);

            const bool isHitSphereBounds = LineSphereIntersection(traceStart, ray.Direction, lengthSqr, compBounds.Origin, compBounds.Radius);
            const bool isPointsInsideBounds =
                (ComputeSquaredDistanceFromBoxToPoint(boxBounds, traceStart) <= sqrRadius) ||
                (ComputeSquaredDistanceFromBoxToPoint(boxBounds, traceEnd) <= sqrRadius);

            if (isHitSphereBounds)
            {
                const auto localStart = component->GetNode()->GetTransform()->GetWorldToLocalMatrix() * traceStart;
                const auto localEnd = component->GetNode()->GetTransform()->GetWorldToLocalMatrix() * traceEnd;

                if (auto smc = ref_cast<StaticMeshRendererComponent>(component))
                {
                    if (auto sm = smc->GetStaticMesh())
                    {
                        for (int sectionIndex = 0; sectionIndex < sm->GetMeshSectionCount(); ++sectionIndex)
                        {
                            auto& section = sm->GetMeshSection(sectionIndex);
                            for (int i = 0; i < section.Indices.size(); i+=3)
                            {
                                auto p0 = section.Vertex[section.Indices[i]];
                                auto p1 = section.Vertex[section.Indices[i+1]];
                                auto p2 = section.Vertex[section.Indices[i+2]];
                                Triangle3f tri {p0.Position, p1.Position, p2.Position};
                                Vector3f intersection{};
                                if (Intersect(tri.GetPlane(), {localStart, localEnd}, intersection) && tri.IsPointIn(intersection))
                                {
                                    result.HitComponent = component;
                                    result.HitNode = component->GetNode();
                                    result.Position = component->GetTransform()->GetLocalToWorldMatrix() * intersection;

                                    return true;

                                    auto bary = tri.BarycentricCoordinates(intersection);
                                    if (bary.x >= 0 && bary.x <= 1 && bary.y >= 0 && bary.y <= 1 && bary.z >= 0 && bary.z <= 1)
                                    {

                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return false;
    }

    void MeshVertexBrush::OnMouseDown(const MouseEventData& e)
    {
        base::OnMouseDown(e);

        if (e.InRegion && e.ButtonId == ImGuiMouseButton_Left)
        {
            const Ray ray = GetWorld()->GetCurrentCamera()->ScreenPointToRay(e.InRegionPosition);

            HitResult result;
            if (this->HitTest(ray, result))
            {
                auto log = std::format("]Hit] name: {}, pos: {}", result.HitComponent->GetNode()->GetName(), to_string(result.Position));
                Logger::Log(log);
            }
        }

    }

    void MeshVertexBrush::Tick(float dt)
    {
        base::Tick(dt);
    }
} // namespace pulsared