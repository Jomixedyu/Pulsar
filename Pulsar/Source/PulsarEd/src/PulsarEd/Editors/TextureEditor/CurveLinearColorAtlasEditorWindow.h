#pragma once
#include "Pulsar/Assets/CurveLinearColorAtlas.h"
#include "TextureEditorWindow.h"

namespace pulsared
{
    class CurveLinearColorAtlasEditorWindow : public TextureEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::CurveLinearColorAtlasEditorWindow, TextureEditorWindow);
        DEFINE_ASSET_EDITOR(CurveLinearColorAtlas, true);
    public:

    };
} // namespace pulsared