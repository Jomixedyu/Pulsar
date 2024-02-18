#pragma once
#include "SceneEditorTool.h"

namespace pulsared
{
    class SceneEditorTransformTool : public SceneEditorTool
    {
        using base = SceneEditorTool;
    public:
        void Begin() override;
        void End() override;
        void Tick(float dt) override;
    };
}