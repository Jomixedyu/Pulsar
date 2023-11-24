#pragma once
#include <PulsarEd/Assembly.h>
#include "MenuEntry.h"
#include "MenuContext.h"
#include "ISubMenu.h"
#include "MenuEntrySubMenu.h"

namespace pulsared
{
    class MenuRenderer
    {
    public:
        static void RenderMenu(ISubMenu* menu, MenuContexts_sp ctxs);
    };
    
    
}