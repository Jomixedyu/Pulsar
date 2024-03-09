#include "AssetDatabase.h"
#include "Pulsar/AssetManager.h"
#include "Shaders/EditorShader.h"

#include <PulsarEd/ToolWindows/ShaderDebugTool.h>

namespace pulsared
{

    ShaderDebugTool::ShaderDebugTool()
    {
    }
    void ShaderDebugTool::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);

        ImGui::BeginTable("tab", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable);
        ImGui::TableSetupColumn("Shader Path");
        ImGui::TableSetupColumn("Latest Date");
        ImGui::TableSetupColumn("IsCompiled");
        ImGui::TableSetupColumn("Compile");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < m_shaderPaths.size(); ++i)
        {
            ImGui::PushID(i);
            auto path = m_shaderPaths[i];
            auto& shader = m_shaders[i];
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text(path.c_str());

            ImGui::TableSetColumnIndex(2);
            string compiled;
            for (auto api : shader->GetSupportedApi())
            {
                compiled += to_string(api);
                compiled += ";";
            }
            ImGui::Text(compiled.c_str());

            ImGui::TableSetColumnIndex(3);
            if (ImGui::Button("Compile"))
            {
                ShaderCompiler::CompileShader(shader.GetPtr());
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
    void ShaderDebugTool::RefreshAssets()
    {
        m_shaderPaths = AssetDatabase::FindAssets(cltypeof<Shader>());
        for (auto& element : m_shaderPaths)
        {
            m_shaders.push_back(GetAssetManager()->LoadAsset<Shader>(element));
        }
    }
    void ShaderDebugTool::OnOpen()
    {
        base::OnOpen();
        RefreshAssets();
    }
} // namespace pulsared