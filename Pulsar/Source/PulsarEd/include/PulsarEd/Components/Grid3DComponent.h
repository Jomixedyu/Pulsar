#pragma once
#include "EditorComponent.h"

namespace pulsared
{
    class Grid3DComponent : public EditorComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::Grid3DComponent, EditorComponent);
    public:
        virtual void OnInitialize() override;
        //virtual void OnDraw() override;
    };
}