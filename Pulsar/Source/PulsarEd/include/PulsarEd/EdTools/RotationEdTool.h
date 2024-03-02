#pragma once
#include "EdTool.h"
#include "SelectorEdTool.h"

namespace pulsared
{

    class RotationEdTool : public SelectorEdTool
    {
        using base = SelectorEdTool;
    public:
        void Begin() override;
    };

} // namespace pulsared
