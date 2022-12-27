#pragma once

namespace apatiteed
{

    class EditorMode
    {

    public:
        virtual void Tick(float dt) = 0;
    };
}