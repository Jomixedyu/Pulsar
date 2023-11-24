#pragma once
#include "EditorWindow.h"
#include <PulsarEd/Menus/MenuEntry.h>

namespace pulsared
{
    class PanelWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::PanelWindow, EditorWindow);
    public:
        virtual void OnOpen() override;
        virtual void OnClose() override;

    protected:
        MenuEntryCheck_sp GetCheckedEntry() const;
    };
    CORELIB_DECL_SHORTSPTR(PanelWindow);
}