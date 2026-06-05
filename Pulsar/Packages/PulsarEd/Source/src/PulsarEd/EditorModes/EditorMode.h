#pragma once
#include <PulsarEd/IEditorTickable.h>

namespace pulsared
{

    class EditorMode : public IEditorTickable
    {

    public:
        virtual void OnEditorTick(float dt) override {};
    };
}