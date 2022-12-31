#pragma once
#include "IEditorTickable.h"

namespace apatiteed
{

    class EditorMode : public IEditorTickable
    {

    public:
        virtual void OnEditorTick(float dt) override {};
    };
}