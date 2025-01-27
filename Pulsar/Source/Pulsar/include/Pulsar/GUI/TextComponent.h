#pragma once
#include "CanvasComponent.h"

namespace pulsar::ui
{
    class TextComponent : public CanvasElement
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ui::TextComponent, CanvasElement);
        CORELIB_CLASS_ATTR(new CategoryAttribute("UI"));
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