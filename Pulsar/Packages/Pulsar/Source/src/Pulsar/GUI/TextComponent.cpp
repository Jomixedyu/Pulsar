#include "GUI/TextComponent.h"

#include "imgui/imgui.h"

namespace pulsar::ui
{

    void TextComponent::OnImmediateRender()
    {
        ImGui::Text(m_text.c_str());
    }

    void TextComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
    }

    void TextComponent::SetText(const string& text)
    {
        m_text = text;
    }

} // namespace pulsar::ui