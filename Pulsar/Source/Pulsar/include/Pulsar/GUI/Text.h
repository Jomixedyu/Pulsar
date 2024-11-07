#pragma once
#include "UIElement.h"

namespace pulsar::ui
{
    class Text : public UIElement
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ui::Text, UIElement);

    public:

        void OnImmediateRender();

        void PostEditChange(FieldInfo* info) override;

        void SetText(const string& text);
        const string& GetText() const { return m_text; }

    protected:
        CORELIB_REFL_DECL_FIELD(m_text);
        string m_text;

        CORELIB_REFL_DECL_FIELD(m_fontSize);
        int m_fontSize = 12;

        CORELIB_REFL_DECL_FIELD(m_color);
        Color4f m_color{};
    };
}