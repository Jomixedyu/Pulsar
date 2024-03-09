#pragma once
#include <PulsarEd/Assembly.h>
#include <CoreLib/Delegate.h>
#include "MenuContext.h"
#include "MenuEntry.h"
#include "ISubMenu.h"

namespace pulsared
{
    using OnGetContext = FunctionDelegate<MenuContextBase_sp>;

    class MenuManager
    {
    public:
        static ISubMenu* GetOrAddMenu(string_view menu_name);
        static ISubMenu* AddMenu(string_view menu_name);
        static void RemoveMenu(string_view menu_name);
        static ISubMenu* GetMenu(string_view menu_name);
        static ISubMenu* GetMainMenu();
        static array_list<ISubMenu*> GetMenus();

        static void RegisterContextProvider(string_view name, const SPtr<OnGetContext>& callback);
        static void UnregisterContextProvider(string_view name, const SPtr<OnGetContext>& callback);
        static MenuContexts_sp RequestContexts(string_view name);
    };
}