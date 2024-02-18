#pragma once

namespace pulsared
{

    class SceneEditorTool
    {
    public:
        virtual ~SceneEditorTool() = default;

        virtual void Begin() {}
        virtual void Tick(float dt) {}
        virtual void End(){}
    };
}