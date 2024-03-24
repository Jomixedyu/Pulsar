#include "MeshVertexBrush.h"

#include "EditorWorld.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"

namespace pulsared
{


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

        // todo

        return false;
    }
} // namespace pulsared