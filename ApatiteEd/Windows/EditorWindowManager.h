#pragma once

#include <vector>
#include "EditorWindow.h"

namespace apatiteed
{
    class EditorWindowManager
    {
    public:
        static EditorWindowManager* GetInstance()
        {
            static EditorWindowManager* instance = new EditorWindowManager;
            return instance;
        }

        static void Reset();

        static void Draw();

        static const std::vector<EditorWindow*>& GetWindows();
        static EditorWindow* GetWindow(string_view name);
    private:
        

    };
}
