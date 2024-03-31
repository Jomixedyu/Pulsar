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

    bool MeshVertexBrush::HitTest(const Ray& ray, HitResult& result)
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

        auto traceStart = ray.Origin;
        auto traceEnd = ray.Origin + ray.Direction * 10000.f;

        auto evec = traceEnd - traceStart;
        auto lengthSqr = Dot(evec, evec);

        for (auto& component : components)
        {
            auto compBounds = component->GetLocalBounds();

            bool isHitBounds = LineSphereIntersection(traceStart, ray.Direction, lengthSqr, compBounds.Origin, compBounds.Radius);
            if (isHitBounds)
            {
                result.HitComponent = component;
                return true;
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
                Logger::Log("Hit: " + result.HitComponent->GetNode()->GetName());
            }
        }

    }

    void MeshVertexBrush::Tick(float dt)
    {
        base::Tick(dt);
    }
} // namespace pulsared