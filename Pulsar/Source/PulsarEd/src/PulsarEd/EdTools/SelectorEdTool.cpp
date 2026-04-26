#include "SelectorEdTool.h"

#include "EditorWorld.h"

#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Raycast.h>
#include <Pulsar/Node.h>
#include <Pulsar/Scene.h>
#include <Pulsar/World.h>
#include <imgui/imgui.h>

namespace pulsared
{

    void SelectorEdTool::Begin()
    {
        base::Begin();
        m_frameSelectorEnabled = false;
        Logger::Log("SelectorTool");
    }

    void SelectorEdTool::Tick(float dt)
    {
        base::Tick(dt);

        // Handle Shift + click/drag separately (ViewEdTool skips OnMouseDown/Up when modifier keys are held)
        if (m_enableSelect && ImGui::IsWindowHovered())
        {
            bool shiftDown = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
            if (shiftDown)
            {
                Vector2f vpPos, vpSize;
                GetViewportSize(vpPos, vpSize);
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    auto pos = ImGui::GetMousePos();
                    m_frameSelectorEnabled = true;
                    // Store as viewport-local
                    m_frameSelectorStartPos = {pos.x - vpPos.x, pos.y - vpPos.y};
                }
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_frameSelectorEnabled)
                {
                    auto curpos = ImGui::GetMousePos();
                    // Convert to viewport-local
                    Vector2f endPos{curpos.x - vpPos.x, curpos.y - vpPos.y};
                    float dist = jmath::Magnitude(endPos - m_frameSelectorStartPos);
                    if (dist < 5.0f)
                    {
                        DoPickSelect(endPos, true);
                    }
                    else
                    {
                        auto rectMin = Vector2f{
                            std::min(m_frameSelectorStartPos.x, endPos.x),
                            std::min(m_frameSelectorStartPos.y, endPos.y)};
                        auto rectMax = Vector2f{
                            std::max(m_frameSelectorStartPos.x, endPos.x),
                            std::max(m_frameSelectorStartPos.y, endPos.y)};
                        FrustumSelect(rectMin, rectMax, true);
                    }
                    m_frameSelectorEnabled = false;
                }
            }
        }

        if (m_frameSelectorEnabled)
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            auto curpos = ImGui::GetMousePos();
            // m_frameSelectorStartPos is viewport-local, convert back to global for ImDrawList
            Vector2f vpPos2, vpSize2;
            GetViewportSize(vpPos2, vpSize2);
            auto startpos = ImVec2{m_frameSelectorStartPos.x + vpPos2.x, m_frameSelectorStartPos.y + vpPos2.y};
            auto fillColor = IM_COL32(100, 160, 255, 46);
            auto borderColor = IM_COL32(100, 160, 255, 200);
            drawList->AddRectFilled(startpos, curpos, fillColor);
            drawList->AddRect(startpos, curpos, borderColor, 0.0f, 0, 1.5f);
        }
    }

    void SelectorEdTool::OnMouseDown(const MouseEventData& e)
    {
        base::OnMouseDown(e);
        if (ImGui::IsWindowHovered() && e.ButtonId == ImGuiMouseButton_Left && m_enableSelect)
        {
            m_frameSelectorEnabled = true;
            m_frameSelectorStartPos = {e.Position.x, e.Position.y};
        }
    }

    void SelectorEdTool::OnMouseUp(const MouseEventData& e)
    {
        base::OnMouseUp(e);
        if (e.ButtonId == ImGuiMouseButton_Left && m_frameSelectorEnabled)
        {
            m_frameSelectorEnabled = false;

            Vector2f endPos = e.Position;
            float dist = jmath::Magnitude(endPos - m_frameSelectorStartPos);

            if (dist < 5.0f)
            {
                // Point pick
                DoPickSelect(endPos, false);
            }
            else
            {
                // Box select
                auto rectMin = Vector2f{
                    std::min(m_frameSelectorStartPos.x, endPos.x),
                    std::min(m_frameSelectorStartPos.y, endPos.y)};
                auto rectMax = Vector2f{
                    std::max(m_frameSelectorStartPos.x, endPos.x),
                    std::max(m_frameSelectorStartPos.y, endPos.y)};
                FrustumSelect(rectMin, rectMax, false);
            }
        }
    }

    ObjectPtr<Node> SelectorEdTool::RaycastSelect(Vector2f screenPos)
    {
        auto cam = m_world->GetCurrentCamera();
        if (!cam) return nullptr;

        // screenPos is already viewport-local (set by ViewEdTool)
        Ray ray = cam->ScreenPointToRay(screenPos);

        ObjectPtr<Node> bestNode = nullptr;
        float bestDist = std::numeric_limits<float>::max();

        for (size_t si = 0; si < m_world->GetSceneCount(); ++si)
        {
            auto scene = m_world->GetScene((int)si);
            if (!scene) continue;
            for (const auto& nodePtr : *scene->GetNodes())
            {
                if (!nodePtr) continue;
                auto bounds = nodePtr->GetBounds();
                // Skip nodes with zero extent (no valid bounding box)
                if (jmath::Magnitude(bounds.Extent) < 1e-5f) continue;

                auto box = bounds.GetBox();
                float dist = 0.0f;
                if (RayUtils::RayAABBIntersect(ray, box, dist) && dist < bestDist)
                {
                    bestDist = dist;
                    bestNode = nodePtr;
                }
            }
        }
        return bestNode;
    }

    void SelectorEdTool::FrustumSelect(Vector2f rectMin, Vector2f rectMax, bool additive)
    {
        auto cam = m_world->GetCurrentCamera();
        if (!cam) return;

        Vector2f vpPos, vpSize;
        GetViewportSize(vpPos, vpSize);
        if (vpSize.x <= 0 || vpSize.y <= 0) return;

        // Build ViewProjection matrix: world -> NDC -> screen
        auto viewMat = cam->GetViewMat();
        auto projMat = cam->GetProjectionMat();
        auto vpMat   = projMat * viewMat;

        if (!additive)
        {
            GetSelection().Clear();
        }

        for (size_t si = 0; si < m_world->GetSceneCount(); ++si)
        {
            auto scene = m_world->GetScene((int)si);
            if (!scene) continue;
            for (auto& nodePtr : *scene->GetNodes())
            {
                if (!nodePtr) continue;
                auto bounds = nodePtr->GetBounds();
                if (jmath::Magnitude(bounds.Extent) < 1e-5f) continue;

                // Project AABB center to screen space
                Vector2f screenPt;
                if (!RayUtils::WorldToScreenPoint(bounds.Origin, vpMat, vpPos, vpSize, screenPt))
                    continue; // Behind camera

                if (screenPt.x >= rectMin.x && screenPt.x <= rectMax.x &&
                    screenPt.y >= rectMin.y && screenPt.y <= rectMax.y)
                {
                    GetSelection().Select(nodePtr);
                }
            }
        }
    }

    void SelectorEdTool::DoPickSelect(Vector2f screenPos, bool additive)
    {
        if (!additive)
        {
            GetSelection().Clear();
        }
        auto hit = RaycastSelect(screenPos);
        if (hit)
        {
            if (additive && GetSelection().IsSelected(hit))
            {
                GetSelection().UnSelect(hit); // Shift-click selected node -> deselect
            }
            else
            {
                GetSelection().Select(hit);
            }
        }
    }

    SelectionSet<Node>& SelectorEdTool::GetSelection()
    {
        return m_world->GetSelection();
    }

} // namespace pulsared