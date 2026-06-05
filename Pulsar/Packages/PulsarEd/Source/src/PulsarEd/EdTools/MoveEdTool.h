#pragma once
#include "EdTool.h"
#include "SelectorEdTool.h"

namespace pulsared
{

    class MoveEdTool : public SelectorEdTool
    {
        using base = SelectorEdTool;
    public:
        void Begin() override;
        void Tick(float dt) override;
    };

} // namespace pulsar