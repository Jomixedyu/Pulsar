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

        void Reset();

        void Draw();

        const std::vector<EditorWindow*>& GetWindows() const;

    private:
        std::vector<EditorWindow*> windows;

    };
}
