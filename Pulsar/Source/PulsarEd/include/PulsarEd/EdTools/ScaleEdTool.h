#pragma once
#include "EdTool.h"
#include "SelectorEdTool.h"

namespace pulsared
{
    class ScaleEdTool : public SelectorEdTool
    {
        using base = SelectorEdTool;
    public:
        void Begin() override;
    };

} // namespace pulsared
