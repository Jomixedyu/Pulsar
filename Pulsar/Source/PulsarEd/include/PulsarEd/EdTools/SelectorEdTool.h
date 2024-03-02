#pragma once
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

        bool m_frameSelector{};
        Vector2f m_frameSelectorStartPos{};
    };

} // namespace pulsared
