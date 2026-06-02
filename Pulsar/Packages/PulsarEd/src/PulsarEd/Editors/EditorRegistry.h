#pragma once
#include "Editors/Editor.h"

namespace pulsared
{
    class EditorRegistry
    {
    public:
        static void Register(Type* editorType);
        static auto& GetRegistrations()
        {
            static array_list<Type*> regs;
            return regs;
        }

        static void Initialize();
        static void Terminate();
        static array_list<SPtr<Editor>>& GetEditors();
        static Editor* GetEditor(Type* editorType);
        template<typename T>
        static T* FindEditor()
        {
            if (auto* editor = GetEditor(cltypeof<T>()))
                return dynamic_cast<T*>(editor);
            return nullptr;
        }
        static void AddEditor(SPtr<Editor> editor);

    private:
        static array_list<SPtr<Editor>> s_editors;
    };
}
