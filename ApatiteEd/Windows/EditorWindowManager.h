#pragma once

#include <vector>
#include "EditorWindow.h"

namespace apatiteed
{
    class EditorWindowManager
    {
    public:
        static void Reset();

        static void Draw();

        static inline Action<EditorWindow*, bool> OnWindowStateChanged;

        static const std::vector<EditorWindow_sp>& GetWindows();
        static EditorWindow_sp GetWindow(string_view name);
        static EditorWindow_sp GetWindow(Type* type);

        //editor window add this
        static void RegisterWindow(EditorWindow_rsp window);
        static void UnRegisterWindow(EditorWindow_rsp window);
    private:
        

    };
}
