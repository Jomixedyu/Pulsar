#pragma once
#include "Assembly.h"

#define DEFINE_EDITOR(EDITOR_TYPE) static inline struct __editor_reg { __editor_reg() \
{ ::pulsared::RegisterEditorType(cltypeof<EDITOR_TYPE>()); } \
} __editor_reg__;

namespace pulsared
{
    class EditorWindow;
    void RegisterEditorType(Type* editorType);

    class Editor : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::Editor, Object);
    public:
        virtual ~Editor() = default;
        virtual void Initialize();
        virtual void Terminate() {}

        virtual string_view GetMenuName() const = 0;

        virtual void RouteInput(const std::vector<uinput::InputEvent>& events) {}

        SPtr<EditorWindow> CreateEditorWindow();
    protected:
        virtual SPtr<EditorWindow> OnCreateEditorWindow() = 0;
    };

} // namespace pulsared
