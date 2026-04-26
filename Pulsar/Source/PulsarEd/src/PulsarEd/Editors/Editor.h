#pragma once
#include "Assembly.h"

namespace pulsared
{
    class EditorWindow;
    class Editor
    {
    public:
        virtual ~Editor() = default;
        virtual void Initialize();
        virtual void Terminate() {}

        virtual string_view GetMenuName() const = 0;

        void RegisterPanelType(Type* type);
        bool ContainsPanelType(Type* type) const;

        SPtr<EditorWindow> CreateEditorWindow();
    protected:
        virtual SPtr<EditorWindow> OnCreateEditorWindow() = 0;
    private:
        array_list<Type*> m_registeredPanelTypes;
    };

} // namespace pulsared
