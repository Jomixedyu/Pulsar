#pragma once
#include "Editors/Editor.h"

namespace pulsared
{
    class AssetEditor : public Editor
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AssetEditor, Editor);
    public:
        AssetEditor() = default;
        
        void Initialize() override {}
        void Terminate() override {}
    };
}
