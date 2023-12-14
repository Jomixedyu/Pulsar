#include "Windows/MainMenuBarWindow.h"

#include "Menus/MenuRenderer.h"

#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Windows/EditorWindowManager.h>

namespace pulsared
{
    void MainMenuBarWindow::OnDrawImGui(float dt)
    {

        if (ImGui::BeginMainMenuBar())
        {
            MenuRenderer::RenderMenu(MenuManager::GetMainMenu(), MenuManager::RequestContexts("Assets"));
            ImGui::EndMainMenuBar();
        }
    }
    MainMenuBarWindow::MainMenuBarWindow()
    {

    }
}