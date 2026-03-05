#pragma once
#include "../../../../Pulsar/include/Pulsar/SelectionSet.h"
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


        SelectionSet<Node>& GetSelection();

        bool m_frameSelectorEnabled{};
        Vector2f m_frameSelectorStartPos{};

        bool m_enableSelect = true;
    };

} // namespace pulsared
