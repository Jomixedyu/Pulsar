#pragma once

#include <vector>
#include "EditorWindow.h"

namespace pulsared
{
    class EditorWindowManager
    {
    public:
        static void Initialize();
        static void Terminate();

        static void Draw();

        static inline Action<EditorWindow*, bool> OnWindowStateChanged;

        static EditorWindow_sp GetOpeningWindow(string_view name);
        static EditorWindow_sp GetOpeningWindow(Type* type);
        static array_list<EditorWindow_sp> GetOpeningWindows(Type* type);

        template <typename T>
        static sptr<T> GetOpeningWindow()
        {
            return sptr_cast<T>(GetOpeningWindow(cltypeof<T>()));
        }

        //editor window add this
        static bool RegisterOpeningWindow(EditorWindow_rsp window);
        static void UnregisterOpeningWindow(EditorWindow_rsp window);

        static void RegisterPanelWindowType(Type* type);
        static void UnregisterPanelWindowType(Type* type);
        static EditorWindow_sp GetPanelWindow(Type* type);
        static EditorWindow_sp GetPanelWindow(string_view name);

    private:


    };
}
