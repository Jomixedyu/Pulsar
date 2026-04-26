#include "ImportAssetWindow.h"

#include <CoreLib.Platform/Window.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>
#include <imgui/imgui.h>

namespace pulsared
{
    static constexpr float k_winW  = 680.f;
    static constexpr float k_winH  = 480.f;
    static constexpr float k_leftW = 180.f;
    static constexpr float k_btnW  = 120.f;

    ImportAssetWindow::ImportAssetWindow(
        array_list<AssetImporterFactory*> factories,
        string targetFolder)
        : ModalDialog("Import Asset")
        , m_factories(std::move(factories))
        , m_targetFolder(std::move(targetFolder))
    {
        if (!m_factories.empty())
            RebuildSettings(0);
    }

    ImVec2 ImportAssetWindow::GetWindowSize() const
    {
        return { k_winW, k_winH };
    }

    void ImportAssetWindow::RebuildSettings(int index)
    {
        m_selectedIndex   = index;
        m_currentSettings = m_factories[index]->CreateImporterSettings();
        m_currentSettings->ImportingTargetFolder = m_targetFolder;
    }

    void ImportAssetWindow::OnDraw(float dt)
    {
        if (m_factories.empty())
        {
            ImGui::TextColored({1, 0.3f, 0.3f, 1}, "No import factories registered.");
            if (ImGui::Button("Close")) m_shouldClose = true;
            return;
        }

        const auto& style    = ImGui::GetStyle();
        const float btnRowH  = ImGui::GetFrameHeight() + style.ItemSpacing.y + 4.f;
        const float contentH = k_winH - ImGui::GetTextLineHeight() - style.WindowPadding.y * 4 - btnRowH - 8.f;
        const float rightW   = k_winW - k_leftW - style.WindowPadding.x * 2 - style.ItemSpacing.x;

        // ─── Left: asset type list ─────────────────────────────────────────────
        ImGui::BeginChild("##TypeList", ImVec2(k_leftW, contentH), true);
        ImGui::TextDisabled("Asset Type");
        ImGui::Separator();
        for (int i = 0; i < (int)m_factories.size(); ++i)
        {
            if (ImGui::Selectable(m_factories[i]->GetDescription().data(), m_selectedIndex == i))
                if (m_selectedIndex != i)
                    RebuildSettings(i);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // ─── Right: import settings via reflection (includes ImportFiles list) ─
        ImGui::BeginChild("##Settings", ImVec2(rightW, contentH), true);
        if (m_currentSettings)
        {
            // ── Add Files button ────────────────────────────────────────────
            if (ImGui::Button("Add Files..."))
            {
                using namespace jxcorlib::platform;
                std::vector<std::filesystem::path> selected;
                // Build filter from current factory's supported extensions
                string filter = "All Files(*.*)|*.*;";
                if (window::OpenFileDialogMulti(window::GetMainWindowHandle(), filter, "", &selected))
                {
                    for (auto& p : selected)
                    {
                        m_currentSettings->ImportFiles->Add(mksptr(new jxcorlib::Path(p)));
                    }
                }
            }
            ImGui::Separator();

            auto* type = m_currentSettings->GetType();
            PImGui::ObjectFieldProperties(type, type, m_currentSettings.get(), nullptr, false);
        }
        ImGui::EndChild();

        // ─── Bottom buttons pinned to bottom-right ─────────────────────────────
        const float btnsW = k_btnW * 2 + style.ItemSpacing.x;
        ImGui::SetCursorPosY(k_winH - ImGui::GetFrameHeight() - style.WindowPadding.y * 2 - 4.f);
        ImGui::Separator();
        ImGui::SetCursorPosX(k_winW - btnsW - style.WindowPadding.x * 2);

        const bool hasFiles = m_currentSettings && !m_currentSettings->ImportFiles->empty();
        if (!hasFiles) ImGui::BeginDisabled();
        if (ImGui::Button("Import", ImVec2(k_btnW, 0)))
        {
            DoImport();
            m_shouldClose = true;
        }
        if (!hasFiles) ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(k_btnW, 0)))
            m_shouldClose = true;
    }

    void ImportAssetWindow::DoImport()
    {
        if (m_factories.empty() || !m_currentSettings) return;
        if (m_currentSettings->ImportFiles->empty())   return;

        m_currentSettings->ImportingTargetFolder = m_targetFolder;
        auto importer = m_factories[m_selectedIndex]->CreateImporter();
        importer->Import(m_currentSettings.get());
    }

} // namespace pulsared
