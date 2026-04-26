#include "Editors/AnimatorControllerEditor/AnimatorControllerEditorWindow.h"
#include "DragInfo.h"
#include <AssetDatabase.h>
#include <Pulsar/IconsForkAwesome.h>
#include <imgui/imgui.h>

namespace pulsared
{
    static const char* kParamTypeNames[] = { "Bool", "Int", "Float", "Trigger" };
    static const char* kOpNames[]        = { "==", "!=", ">", "<", "Trigger" };

    static bool DrawAssetPicker(const char* label, pulsar::RCPtr<pulsar::AssetObject>& outPtr, Type* assetType)
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "%s", outPtr ? outPtr->GetName().c_str() : "(None)");

        float w = ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth(w - 22.f);
        ImGui::InputText(label, buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

        bool changed = false;
        if (ImGui::BeginDragDropTarget())
        {
            if (auto* payload = ImGui::GetDragDropPayload())
            {
                if (string_view(payload->DataType) == AssetObjectDragInfo::Name)
                {
                    auto* drag = static_cast<AssetObjectDragInfo*>(payload->Data);
                    if (drag->Type->IsSubclassOf(assetType))
                    {
                        if (ImGui::AcceptDragDropPayload(AssetObjectDragInfo::Name.data()))
                        {
                            outPtr = cast<pulsar::AssetObject>(AssetDatabase::LoadAssetById(drag->AssetGuid));
                            changed = true;
                        }
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(ICON_FK_TIMES "##clr")) { outPtr.Reset(); changed = true; }
        return changed;
    }

    void AnimatorControllerEditorWindow::OnOpen()
    {
        base::OnOpen();
        m_controller = cast<pulsar::AnimatorController>(m_assetObject);
    }

    void AnimatorControllerEditorWindow::OnDrawAssetEditor(float dt)
    {
        base::OnDrawAssetEditor(dt);
        if (!m_controller) return;

        const float totalW = ImGui::GetContentRegionAvail().x;
        const float totalH = ImGui::GetContentRegionAvail().y;

        ImGui::BeginChild("##params", { totalW * 0.26f, totalH }, ImGuiChildFlags_Border);
        DrawParamsPanel();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##states", { totalW * 0.35f, totalH }, ImGuiChildFlags_Border);
        DrawStatesPanel();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##trans", { 0, totalH }, ImGuiChildFlags_Border);
        DrawTransitionsPanel();
        ImGui::EndChild();
    }

    // -----------------------------------------------------------------------
    // Parameters Panel
    // -----------------------------------------------------------------------
    void AnimatorControllerEditorWindow::DrawParamsPanel()
    {
        ImGui::TextColored({0.6f,0.8f,1.f,1.f}, ICON_FK_SLIDERS " Parameters");
        ImGui::Separator();

        auto& params = const_cast<array_list<pulsar::AnimatorParam>&>(m_controller->GetParams());
        int   removeIdx = -1;

        for (int i = 0; i < (int)params.size(); ++i)
        {
            auto& p = params[i];
            ImGui::PushID(i);
            ImGui::TextDisabled("[%s]", kParamTypeNames[(int)p.Type]);
            ImGui::SameLine();
            ImGui::TextUnformatted(p.Name.c_str());
            ImGui::SameLine();
            ImGui::SetNextItemWidth(50.f);
            bool dirty = false;
            switch (p.Type)
            {
            case pulsar::AnimatorParamType::Bool:
            case pulsar::AnimatorParamType::Trigger:
                dirty = ImGui::Checkbox("##v", &p.BoolValue); break;
            case pulsar::AnimatorParamType::Int:
                dirty = ImGui::InputInt("##v", &p.IntValue, 0); break;
            case pulsar::AnimatorParamType::Float:
                dirty = ImGui::InputFloat("##v", &p.FloatValue, 0.f, 0.f, "%.2f"); break;
            }
            if (dirty) AssetDatabase::MarkDirty(m_assetObject);
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FK_TIMES "##dp")) removeIdx = i;
            ImGui::PopID();
        }
        if (removeIdx >= 0)
        {
            m_controller->RemoveParam(params[removeIdx].Name);
            AssetDatabase::MarkDirty(m_assetObject);
        }

        ImGui::Separator();
        // 输入框独占一行，类型选择独占一行，按钮独占一行——更宽敞
        ImGui::Text("Name:");
        ImGui::SetNextItemWidth(-1.f);
        ImGui::InputText("##pname", m_addParamName, sizeof(m_addParamName));
        ImGui::Text("Type:");
        ImGui::SetNextItemWidth(-1.f);
        ImGui::Combo("##pt", &m_addParamType, kParamTypeNames, 4);
        if (ImGui::Button(ICON_FK_PLUS " Add Param", {-1.f, 0.f}) && m_addParamName[0])
        {
            pulsar::AnimatorParam p;
            p.Name = m_addParamName;
            p.Type = (pulsar::AnimatorParamType)m_addParamType;
            m_controller->AddParam(p);
            m_addParamName[0] = '\0';
            AssetDatabase::MarkDirty(m_assetObject);
        }
    }

    // -----------------------------------------------------------------------
    // States Panel
    // -----------------------------------------------------------------------
    void AnimatorControllerEditorWindow::DrawStatesPanel()
    {
        ImGui::TextColored({0.6f,1.f,0.7f,1.f}, ICON_FK_CIRCLE " States");
        ImGui::Separator();

        auto& states    = const_cast<array_list<pulsar::AnimatorState>&>(m_controller->GetStates());
        auto& allStates = m_controller->GetStates();
        int   removeIdx = -1;

        for (int i = 0; i < (int)states.size(); ++i)
        {
            auto& st       = states[i];
            bool  isDef    = st.Name == m_controller->GetDefaultState();
            ImGui::PushID(i);

            if (isDef) ImGui::PushStyleColor(ImGuiCol_Text, {1.f, 0.85f, 0.2f, 1.f});
            bool sel = (m_selectedStateIdx == i);
            ImGui::Selectable(st.Name.c_str(), sel);
            if (isDef) ImGui::PopStyleColor();

            // 右键菜单
            if (ImGui::BeginPopupContextItem("##stctx"))
            {
                m_selectedStateIdx = i;
                if (!isDef && ImGui::MenuItem(ICON_FK_STAR " Set as Default"))
                {
                    m_controller->SetDefaultState(st.Name);
                    AssetDatabase::MarkDirty(m_assetObject);
                }
                // Make Transition To → 子菜单
                if (ImGui::BeginMenu(ICON_FK_LONG_ARROW_RIGHT " Make Transition To"))
                {
                    for (auto& dst : allStates)
                    {
                        if (dst.Name == st.Name) continue;
                        if (ImGui::MenuItem(dst.Name.c_str()))
                        {
                            pulsar::AnimatorTransition tr;
                            tr.FromState = st.Name;
                            tr.ToState   = dst.Name;
                            m_controller->AddTransition(std::move(tr));
                            m_selectedTransIdx = (int)m_controller->GetTransitions().size() - 1;
                            AssetDatabase::MarkDirty(m_assetObject);
                        }
                    }
                    string anyLabel = "Any State -> " + st.Name;
                    if (ImGui::MenuItem(anyLabel.c_str()))
                    {
                        pulsar::AnimatorTransition tr;
                        tr.FromState = "";
                        tr.ToState   = st.Name;
                        m_controller->AddTransition(std::move(tr));
                        m_selectedTransIdx = (int)m_controller->GetTransitions().size() - 1;
                        AssetDatabase::MarkDirty(m_assetObject);
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                if (ImGui::MenuItem(ICON_FK_TRASH " Delete"))
                {
                    removeIdx = i;
                    if (m_selectedStateIdx == i) m_selectedStateIdx = -1;
                }
                ImGui::EndPopup();
            }

            if (ImGui::IsItemClicked()) m_selectedStateIdx = i;

            ImGui::PopID();
        }
        if (removeIdx >= 0)
        {
            m_controller->RemoveState(states[removeIdx].Name);
            AssetDatabase::MarkDirty(m_assetObject);
        }

        // ---- 选中 State 属性 ----
        if (m_selectedStateIdx >= 0 && m_selectedStateIdx < (int)states.size())
        {
            auto& st = states[m_selectedStateIdx];
            ImGui::Separator();
            bool isDefSel = st.Name == m_controller->GetDefaultState();
            ImGui::TextColored({1.f,0.9f,0.4f,1.f}, ICON_FK_CIRCLE_O " %s", st.Name.c_str());
            if (isDefSel) { ImGui::SameLine(); ImGui::TextDisabled("(Default)"); }

            ImGui::SetNextItemWidth(-1.f);
            if (ImGui::SliderFloat("Speed##spd", &st.Speed, 0.1f, 5.f, "%.2f"))
                AssetDatabase::MarkDirty(m_assetObject);

            if (ImGui::Checkbox("Loop##lp", &st.Loop))
                AssetDatabase::MarkDirty(m_assetObject);

            ImGui::Text("Clip:  (drag from Content Browser)");
            pulsar::RCPtr<pulsar::AssetObject> clipAsset = cast<pulsar::AssetObject>(st.Clip);
            if (DrawAssetPicker("##clip", clipAsset, cltypeof<pulsar::AnimationClip>()))
            {
                st.Clip = cast<pulsar::AnimationClip>(clipAsset);
                AssetDatabase::MarkDirty(m_assetObject);
            }
        }

        // ---- 新建 State ----
        ImGui::Separator();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 28.f);
        ImGui::InputText("##sname", m_addStateName, sizeof(m_addStateName));
        ImGui::SameLine();
        if (ImGui::Button(ICON_FK_PLUS "##adds") && m_addStateName[0])
        {
            pulsar::AnimatorState st;
            st.Name = m_addStateName;
            m_controller->AddState(std::move(st));
            m_selectedStateIdx = (int)m_controller->GetStates().size() - 1;
            m_addStateName[0] = '\0';
            AssetDatabase::MarkDirty(m_assetObject);
        }
    }

    // -----------------------------------------------------------------------
    // Transitions Panel
    // -----------------------------------------------------------------------
    void AnimatorControllerEditorWindow::DrawTransitionsPanel()
    {
        ImGui::TextColored({1.f,0.75f,0.4f,1.f}, ICON_FK_LONG_ARROW_RIGHT " Transitions");
        ImGui::Separator();

        auto& transitions = const_cast<array_list<pulsar::AnimatorTransition>&>(m_controller->GetTransitions());
        int   removeIdx   = -1;

        for (int i = 0; i < (int)transitions.size(); ++i)
        {
            auto& tr = transitions[i];
            ImGui::PushID(i);
            string label = std::format("{} -> {}",
                tr.FromState.empty() ? "Any" : tr.FromState, tr.ToState);
            bool sel = (m_selectedTransIdx == i);
            ImGui::Selectable(label.c_str(), sel);
            if (ImGui::IsItemClicked()) m_selectedTransIdx = i;

            if (ImGui::BeginPopupContextItem("##trctx"))
            {
                m_selectedTransIdx = i;
                if (ImGui::MenuItem(ICON_FK_TRASH " Delete")) removeIdx = i;
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }
        if (removeIdx >= 0)
        {
            if (m_selectedTransIdx == removeIdx) m_selectedTransIdx = -1;
            m_controller->RemoveTransition(removeIdx);
            AssetDatabase::MarkDirty(m_assetObject);
        }

        // ---- 选中 Transition 详情 ----
        if (m_selectedTransIdx >= 0 && m_selectedTransIdx < (int)transitions.size())
        {
            auto& tr = transitions[m_selectedTransIdx];
            ImGui::Separator();
            ImGui::TextColored({1.f,0.85f,0.5f,1.f}, "%s -> %s",
                tr.FromState.empty() ? "Any" : tr.FromState.c_str(), tr.ToState.c_str());

            if (ImGui::Checkbox("Has Exit Time##het", &tr.HasExitTime))
                AssetDatabase::MarkDirty(m_assetObject);
            if (tr.HasExitTime)
            {
                ImGui::SetNextItemWidth(-1.f);
                if (ImGui::SliderFloat("Exit Time##et", &tr.ExitTime, 0.f, 1.f, "%.2f"))
                    AssetDatabase::MarkDirty(m_assetObject);
            }

            // Conditions 表格
            ImGui::Text("Conditions:");
            int removeCond = -1;
            if (ImGui::BeginTable("##conds", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("Param",     ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Op",        ImGuiTableColumnFlags_WidthFixed, 44.f);
                ImGui::TableSetupColumn("Threshold", ImGuiTableColumnFlags_WidthFixed, 60.f);
                ImGui::TableSetupColumn("##del",     ImGuiTableColumnFlags_WidthFixed, 20.f);
                ImGui::TableHeadersRow();

                for (int ci = 0; ci < (int)tr.Conditions.size(); ++ci)
                {
                    auto& cond = tr.Conditions[ci];
                    ImGui::TableNextRow();
                    ImGui::PushID(ci);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(cond.ParamName.c_str());

                    ImGui::TableSetColumnIndex(1);
                    int opIdx = (int)cond.Op;
                    ImGui::SetNextItemWidth(-1.f);
                    if (ImGui::Combo("##op", &opIdx, kOpNames, 5))
                    { cond.Op = (pulsar::AnimatorConditionOp)opIdx; AssetDatabase::MarkDirty(m_assetObject); }

                    ImGui::TableSetColumnIndex(2);
                    ImGui::SetNextItemWidth(-1.f);
                    if (ImGui::InputFloat("##th", &cond.Threshold, 0.f, 0.f, "%.2f"))
                        AssetDatabase::MarkDirty(m_assetObject);

                    ImGui::TableSetColumnIndex(3);
                    if (ImGui::SmallButton(ICON_FK_TIMES "##rc")) removeCond = ci;

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            if (removeCond >= 0)
            {
                tr.Conditions.erase(tr.Conditions.begin() + removeCond);
                AssetDatabase::MarkDirty(m_assetObject);
            }

            // 添加条件
            auto& params = m_controller->GetParams();
            if (!params.empty())
            {
                array_list<const char*> pnames;
                for (auto& p : params) pnames.push_back(p.Name.c_str());
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 80.f);
                ImGui::Combo("##cparam", &m_addCondParamIdx, pnames.data(), (int)pnames.size());
                ImGui::SameLine();
                if (ImGui::Button(ICON_FK_PLUS " Condition"))
                {
                    if (m_addCondParamIdx < (int)params.size())
                    {
                        pulsar::AnimatorCondition cond;
                        cond.ParamName = params[m_addCondParamIdx].Name;
                        cond.Op = (params[m_addCondParamIdx].Type == pulsar::AnimatorParamType::Trigger)
                            ? pulsar::AnimatorConditionOp::Trigger
                            : pulsar::AnimatorConditionOp::Equals;
                        tr.Conditions.push_back(cond);
                        AssetDatabase::MarkDirty(m_assetObject);
                    }
                }
            }
            else
            {
                ImGui::TextDisabled("(Add parameters first)");
            }
        }
    }

} // namespace pulsared
