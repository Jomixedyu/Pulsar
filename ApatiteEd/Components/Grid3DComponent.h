#pragma once
#include "EditorComponent.h"

namespace apatiteed
{
    class Grid3DComponent : public EditorComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::Grid3DComponent, EditorComponent);
    public:
        virtual void OnInitialize() override;
        virtual void OnDraw() override;
    };
}