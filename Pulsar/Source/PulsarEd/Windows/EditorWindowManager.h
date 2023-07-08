#pragma once

#include <vector>
#include "EditorWindow.h"

namespace pulsared
{
    class EditorWindowManager
    {
    public:
        static void Reset();

        static void Draw();

        static inline Action<EditorWindow*, bool> OnWindowStateChanged;

        static EditorWindow_sp GetWindow(string_view name);
        static EditorWindow_sp GetWindow(Type* type);
        template<typename T>
        static sptr<T> GetWindow()
        {
            return sptr_cast<T>(GetWindow(cltypeof<T>()));
        }
        //editor window add this
        static bool RegisterWindow(EditorWindow_rsp window);
        static void UnRegisterWindow(EditorWindow_rsp window);

    private:
        

    };
}
