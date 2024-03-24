#pragma once
#include "EdTool.h"
#include "ViewEdTool.h"

namespace pulsared
{
    class BrushEdTool : public ViewEdTool
    {
        using base = ViewEdTool;
    public:
        virtual void Begin() override;
    };

} // namespace pulsared
