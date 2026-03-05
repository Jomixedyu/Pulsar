#pragma once

#include "EdGuiWindow.h"
#include <vector>

namespace pulsared
{
    class EditorWindowManager
    {
    public:
        static void Initialize();
        static void Terminate();

        static void Draw(float dt);

        static inline Action<EdGuiWindow*, bool> OnWindowStateChanged;

        static EdGuiWindow_sp GetOpeningWindow(string_view name);
        static EdGuiWindow_sp GetOpeningWindow(Type* type);
        static array_list<EdGuiWindow_sp> GetOpeningWindows(Type* type);

        template <typename T>
        static SPtr<T> GetOpeningWindow()
        {
            return sptr_cast<T>(GetOpeningWindow(cltypeof<T>()));
        }

        //editor window add this
        static bool RegisterOpeningWindow(EdGuiWindow_rsp window);
        static void UnregisterOpeningWindow(EdGuiWindow_rsp window);


    };
}
