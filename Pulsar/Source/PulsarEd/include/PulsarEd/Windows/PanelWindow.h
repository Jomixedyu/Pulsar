#pragma once
#include "EdGuiWindow.h"
#include <PulsarEd/Menus/MenuEntry.h>

namespace pulsared
{
    class PanelWindow : public EdGuiWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::PanelWindow, EdGuiWindow);
    public:
        virtual void OnOpen() override;
        virtual void OnClose() override;
        int GetParentWindowId() const { return m_parentWindowId; }

        ImGuiWindowClass GetGuiWindowClass() const override;

        int m_parentWindowId;
    protected:
        MenuEntryCheck_sp GetCheckedEntry() const;
    };
    CORELIB_DECL_SHORTSPTR(PanelWindow);
}