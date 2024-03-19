#pragma once
#include "EdTool.h"
#include "SelectionSet.h"
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
        SelectionSet<Node>& GetSelection();

        bool m_frameSelector{};
        Vector2f m_frameSelectorStartPos{};
    protected:
        bool m_enableSelect = true;
    };

} // namespace pulsared
