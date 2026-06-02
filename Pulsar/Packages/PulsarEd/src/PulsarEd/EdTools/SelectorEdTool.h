#pragma once
#include <Pulsar/SelectionSet.h>
#include "EdTool.h"
#include "ViewEdTool.h"

namespace pulsared
{
    class SelectorEdTool : public ViewEdTool
    {
        using base = ViewEdTool;
    public:
        void Begin() override;
        void Tick(float dt) override;

    protected:
        void OnMouseDown(const MouseEventData& e) override;
        void OnMouseUp(const MouseEventData& e) override;

        // Point pick: ray vs node AABB, returns the closest hit node
        ObjectPtr<Node> RaycastSelect(Vector2f screenPos);

        // Box select: project node AABB centers to screen space, collect nodes inside the rect
        void FrustumSelect(Vector2f rectMin, Vector2f rectMax, bool additive);

        // Single pick (clears selection first if not additive)
        void DoPickSelect(Vector2f screenPos, bool additive);

        SelectionSet<Node>& GetSelection();

        bool m_frameSelectorEnabled{};
        Vector2f m_frameSelectorStartPos{};   // absolute screen position
        bool m_enableSelect = true;
    };

} // namespace pulsared