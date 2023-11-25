#include "CoreLib.Platform/FolderWatch.h"
#include "CoreLib.Platform/Window.h"
#include "Importers/AssetImporter.h"

#include <PulsarEd/Windows/WorkspaceWindow.h>
#include <PulsarEd/AssetDatabase.h>
#include <PulsarEd/Workspace.h>
#include <Pulsar/IconsForkAwesome.h>
#include <PulsarEd/AssetDatabase.h>
#include <PulsarEd/AssetProviders/AssetProvider.h>
#include <PulsarEd/Menus/Types.h>

#include <PulsarEd/Utils/AssetUtil.h>

#include <imgui/imgui_internal.h>
#include <PulsarEd/Menus/MenuRenderer.h>

namespace pulsared
{
    namespace PImGui
    {
        static bool FileButton(const char* str,
            ImTextureID texture_id, ImVec2 size, float label_height, bool selected, bool is_dirty, ImTextureID dirty_texid,
            void* user_data, const std::function<void(void*)>& tootip,
            ImVec2 uv0 = ImVec2(0, 0), ImVec2 uv1 = ImVec2(1, 1)
        )
        {
            using namespace ImGui;

            ImGuiContext& g = *GImGui;
            ImGuiWindow* window = GetCurrentWindow();
            if (window->SkipItems)
                return false;

            ImGuiID id = window->GetID(str);

            const ImVec2 padding = g.Style.FramePadding;

            ImVec2 offset = size;
            offset.y += label_height;

            ImRect pic_bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2.0f);
            ImRect dirty_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2{ 16,16 });

            const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + offset + padding * 2.0f);
            ItemSize(bb);
            if (!ItemAdd(bb, id))
                return false;

            bool hovered{}, held{};
            bool pressed{};

            pressed = ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnDoubleClick);

            ImVec4 bg_col = { 0,0,0,0 };
            ImVec4 tint_col = { 1,1,1,1 };


            // Render
            ImU32 col{};
            if (selected)
            {
                col = GetColorU32(ImGuiCol_ButtonHovered);
            }
            else
            {
                col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            }
            RenderNavHighlight(bb, id);
            RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));

            if (bg_col.w > 0.0f)
                window->DrawList->AddRectFilled(bb.Min + padding, bb.Max - padding, GetColorU32(bg_col));
            window->DrawList->AddImage(texture_id, pic_bb.Min + padding, pic_bb.Max - padding, uv0, uv1, GetColorU32(tint_col));

            if (is_dirty)
            {
                window->DrawList->AddImage(dirty_texid, dirty_bb.Min + padding, dirty_bb.Max,
                    uv0, uv1, GetColorU32(tint_col));
            }

            auto label_pos = bb.Min + padding;
            label_pos.y += size.y;
            RenderText(label_pos, str);

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
            {
                if (tootip)
                {
                    //SetNextWindowSize({ 300, -FLT_MIN });
                    if (BeginTooltip())
                    {
                        tootip(user_data);
                        EndTooltip();
                    }
                }
            }

            return pressed;
        }
    }


    void WorkspaceWindow::RenderFolderTree(sptr<AssetFileNode> node)
    {
        ImGuiTreeNodeFlags base_flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanFullWidth;

        if (!node->IsFolder)
        {
            return;
        }

        if (node->IsCollapsed)
        {
            base_flags |= ImGuiTreeNodeFlags_DefaultOpen;
        }

        if (m_currentFolder == node->AssetPath)
        {
            base_flags |= ImGuiTreeNodeFlags_Selected;
        }


        bool isOpened = ImGui::TreeNodeEx(node->AssetName.c_str(), base_flags);

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            SetCurrentFolder(node->AssetPath);
        }

        if (isOpened)
        {
            if (node->IsFolder)
            {
                for (auto& i : node->Children)
                {
                    RenderFolderTree(i);
                }
            }

        }

        if (isOpened)
        {
            ImGui::TreePop();

        }
    }
    void WorkspaceWindow::OnOpen()
    {
        base::OnOpen();
    }
    void WorkspaceWindow::OnClose()
    {
        base::OnClose();
    }

    void WorkspaceWindow::OnDrawImGui()
    {
        if (!Workspace::IsOpened())
        {
            ImGui::Text("no project");
            return;
        }

        OnDrawBar();

        ImGui::Columns(2);

        if (!m_layoutColumnOffset.has_value())
        {
            m_layoutColumnOffset = ImGui::GetWindowWidth() * 0.3f;
        }
        else
        {
            m_layoutColumnOffset = ImGui::GetColumnOffset(1);
        }

        ImGui::SetColumnOffset(1, m_layoutColumnOffset.value());

        OnDrawFolderTree();

        ImGui::NextColumn();

        OnDrawContent();

        ImGui::Columns(1);

    }

    static void InitMenu()
    {
        auto menu = MenuManager::GetOrAddMenu("Assets");

        auto createAssetMenu = mksptr(new MenuEntrySubMenu("CreateAsset", "Create Asset"));

        auto onCanCreate = MenuCanOperate::FromLambda([](MenuContexts_sp ctxs) -> bool {
            {
                if (WorkspaceWindowMenuContext_sp ctx; ctxs && (ctx = ctxs->FindContext<WorkspaceWindowMenuContext>()))
                {
                    if (!ctx->CurrentPath.empty())
                        return true;
                }
                return false;
            }}
        );

        auto onAssetCreated = MenuAction::FromLambda([](MenuContexts_sp ctxs) {
            if (WorkspaceWindowMenuContext_sp ctx; ctxs && (ctx = ctxs->FindContext<WorkspaceWindowMenuContext>()))
            {
                Type* type = AssemblyManager::GlobalFindType(ctxs->EntryName);
                AssetDatabase::NewAsset(ctx->CurrentPath, type);
                Logger::Log("create asset : " + ctx->CurrentPath + " ; " + type->GetName());
            }}
        );

        for (auto& type : AssemblyManager::GlobalSearchType(cltypeof<AssetObject>()))
        {
            auto typePaths = StringUtil::Split(type->GetName(), "::");
            MenuEntrySubMenu_sp submenu = createAssetMenu;
            for (size_t i = 0; i < typePaths.size(); i++)
            {

                auto friendlyName = StringUtil::FriendlyName(typePaths[i]);
                if (i == typePaths.size() - 1)
                {
                    auto btn = mksptr(new MenuEntryButton(type->GetName(), friendlyName));
                    btn->Action = onAssetCreated;
                    btn->CanOperate = onCanCreate;
                    submenu->AddEntry(btn);
                }
                else
                {
                    auto name = typePaths[i];
                    MenuEntrySubMenu_sp newPathComponent = submenu->FindSubMenuEntry(name);
                    if (!newPathComponent)
                    {
                        newPathComponent = mksptr(new MenuEntrySubMenu(name, friendlyName));
                        submenu->AddEntry(newPathComponent);
                    }
                    submenu = newPathComponent;
                }
            }
        }

        menu->AddEntry(createAssetMenu);

        auto createFolderBtn = mksptr(new MenuEntryButton("Create Folder"));
        createFolderBtn->CanOperate = onCanCreate;
        //createFolderBtn->Action = MenuAction::FromLambda();
        menu->AddEntry(createFolderBtn);


        auto assetCtx = MenuManager::GetOrAddMenu("WorkspaceWindow.AssetsContext");
        assetCtx->AddEntry(mksptr(new MenuEntrySeparate("Create")));
        assetCtx->AddEntry(createAssetMenu);
        assetCtx->AddEntry(createFolderBtn);

        assetCtx->AddEntry(mksptr(new MenuEntrySeparate("Common")));
        assetCtx->AddEntry(mksptr(new MenuEntryButton("Rename")));
        assetCtx->AddEntry(mksptr(new MenuEntryButton("Duplicate")));
        assetCtx->AddEntry(mksptr(new MenuEntryButton("Save")));
        assetCtx->AddEntry(mksptr(new MenuEntryButton("Delete")));

        assetCtx->AddEntry(mksptr(new MenuEntrySeparate("Assets")));
        {
            auto entry = mksptr(new MenuEntryButton("Reload"));
            entry->Action = MenuAction::FromLambda([](MenuContexts_sp ctxs) {
                {
                    if (WorkspaceWindowMenuContext_sp ctx; ctxs && (ctx = ctxs->FindContext<WorkspaceWindowMenuContext>()))
                    {
                        for (auto& item : ctx->SelectedFiles)
                        {
                            if (auto file = item.lock())
                            {
                                if (!file->IsFolder)
                                {
                                    AssetDatabase::ReloadAsset(file->AssetMeta->Handle);
                                }
                            }
                            
                        }
                    }
                }});
            assetCtx->AddEntry(entry);
        }


        assetCtx->AddEntry(mksptr(new MenuEntrySeparate("Explorer")));
        assetCtx->AddEntry(mksptr(new MenuEntryButton("ShowExplorer", "Show In Explorer")));

    }

    void WorkspaceWindow::ClearSelectedFile()
    {
        m_selectedFiles.clear();
    }
    void WorkspaceWindow::OnCurrentFolderChanged()
    {
        ClearSelectedFile();
    }
    void WorkspaceWindow::SetCurrentFolder(string_view path)
    {
        m_currentFolder = path;
        OnCurrentFolderChanged();
    }

    void WorkspaceWindow::OnDrawBar()
    {
        static bool a = false;
        if (a == false)
        {
            InitMenu();
            a = true;
        }
        if (ImGui::BeginMenuBar())
        {

            if (ImGui::Button(ICON_FK_PLUS "  Add Asset"))
            {
                ImGui::OpenPopup("WorkspaceWindow.AddAsset");
            }
            if (ImGui::BeginPopup("WorkspaceWindow.AddAsset"))
            {
                ImGui::SeparatorText("Add Asset");

                auto menu = MenuManager::GetMenu("Assets");
                auto ctxs = mksptr(new MenuContexts());
                ctxs->Contexts.push_back(MakeMenuContext());
                MenuRenderer::RenderMenu(menu->FindSubMenuEntry("CreateAsset").get(), ctxs);

                ImGui::EndPopup();
            }

            if (ImGui::Button(ICON_FK_DOWNLOAD "  Import"))
            {
                const auto mainWindow = jxcorlib::platform::window::GetMainWindowHandle();
                string selectedFileName;

                string filterStr;
                for (const auto factory : AssetImporterFactoryManager::GetFactories())
                {
                    filterStr += StringUtil::Concat(factory->GetDescription(), "(", factory->GetFilter() , ")", ";", factory->GetFilter(), ";");
                }

                if(jxcorlib::platform::window::OpenFileDialog(mainWindow, filterStr, "", &selectedFileName))
                {

                }
            }

            if (ImGui::Button(ICON_FK_FOLDER_OPEN "  Explorer"))
            {

            }


            ImGui::Text("Path:  ");
            auto paths = StringUtil::Split(m_currentFolder, u8char("/"));
            ImGui::PushStyleColor(ImGuiCol_Button, {});
            array_list<string> trackPath;
            for (size_t i = 0; i < paths.size(); i++)
            {
                trackPath.push_back(paths[i]);
                ImGui::PushID(i);
                if (ImGui::Button(paths[i].c_str()))
                {
                    SetCurrentFolder(StringUtil::Join(trackPath, "/"));
                }
                ImGui::Text("/");
                ImGui::PopID();
            }

            ImGui::PopStyleColor();

            ImGui::EndMenuBar();
        }
    }

    void WorkspaceWindow::OnDrawFolderTree()
    {
        if (ImGui::BeginChild("FolderTree"))
        {
            RenderFolderTree(AssetDatabase::FileTree);
        }
        ImGui::EndChild();
    }

    void WorkspaceWindow::OnDrawContent()
    {
        ImGui::InputTextWithHint("##Search", "Search", this->search_buf, sizeof(this->search_buf));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SliderFloat("##IconSize", &m_iconSize, 30, 300);

        if (ImGui::BeginChild("FilesViewer"))
        {
            RenderFileContent();
        }
        ImGui::EndChild();
    }

    static void _RenderFileToolTips(void* data)
    {
        auto node = (AssetFileNode*)data;

        Type* assetType = node->GetAssetType();


        auto title = StringUtil::Concat(StringUtil::FriendlyName(node->AssetName), " ( ", StringUtil::FriendlyName(assetType->GetShortName()), " ) ");
        ImGui::SeparatorText(title.c_str());

        ImGui::Text(("Type: " + assetType->GetName()).c_str());

        ImGui::Text(("Path: " + node->AssetPath).c_str());
        ImGui::Text(("DiskPath: " + node->PhysicsPath.string()).c_str());

    }

    template<typename IT, typename V>
    static IT weaks_find(IT begin, IT end, V value)
    {
        for (auto it = begin; it != end; ++it)
        {
            if (it->lock() == value.lock())
            {
                return it;
            }
        }
        return end;
    }

    void WorkspaceWindow::RenderFileContent()
    {
        std::shared_ptr<AssetFileNode> p = AssetDatabase::FileTree->Find(m_currentFolder);

        static float padding = 16.f;
        float cellSize = m_iconSize + padding;
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        for (auto& child : p->Children)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, {});

            Type* assetType = child->GetAssetType();
            gfx::GFXDescriptorSet_wp descSet = AssetDatabase::IconPool->GetDescriptorSet(assetType);
            gfx::GFXDescriptorSet_wp dirtySet = AssetDatabase::IconPool->GetDescriptorSet("WorkspaceWindow.Dirty"_idxstr);

            auto isSelected = weaks_find(m_selectedFiles.begin(), m_selectedFiles.end(), std::weak_ptr{ child }) != m_selectedFiles.end();
            bool isDirty = child->IsFolder ? false : AssetDatabase::IsDirty(child->AssetMeta->Handle);


            if (PImGui::FileButton(child->AssetName.c_str(), (void*)descSet.lock()->GetId(), ImVec2(m_iconSize, m_iconSize),
                /*label */30, isSelected, isDirty, (void*)dirtySet.lock()->GetId(), child.get(), &_RenderFileToolTips))
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (child->IsFolder)
                    {
                        SetCurrentFolder(child->AssetPath);
                    }
                    else
                    {
                        Logger::Log("open asset editor: " + assetType->GetName(), LogLevel::Info);
                        AssetUtil::OpenAssetEditor(AssetDatabase::LoadAssetAtPath(child->AssetPath));
                    }
                }
                else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    if (ImGui::GetIO().KeyShift)
                    {
                        auto it = weaks_find(m_selectedFiles.begin(), m_selectedFiles.end(), std::weak_ptr{ child });
                        if (it != m_selectedFiles.end())
                        {
                            m_selectedFiles.erase(it);
                        }
                        else
                        {
                            m_selectedFiles.push_back(child);
                        }
                    }
                    else
                    {
                        m_selectedFiles.clear();
                        m_selectedFiles.push_back(child);
                    }
                }


            }


            ImGui::PopStyleColor();

            //if (isDrawing)
            ImGui::NextColumn();

        }
        ImGui::Columns(1);


        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup("WorkspaceWindow.AssetContext");
        }
        if (ImGui::BeginPopup("WorkspaceWindow.AssetContext"))
        {
            auto menu = MenuManager::GetMenu("WorkspaceWindow.AssetsContext");

            auto ctxs = mksptr(new MenuContexts());
            ctxs->Contexts.push_back(MakeMenuContext());
            MenuRenderer::RenderMenu(menu.get(), ctxs);

            ImGui::EndPopup();
        }


    }
    WorkspaceWindowMenuContext_sp WorkspaceWindow::MakeMenuContext()
    {
        auto ctx = mksptr(new WorkspaceWindowMenuContext{ m_currentFolder });
        ctx->CurrentPath = m_currentFolder;
        ctx->SelectedFiles = m_selectedFiles;
        return ctx;
    }

    WorkspaceWindow::WorkspaceWindow()
    {
        ::memset(this->search_buf, 0, sizeof(this->search_buf));
    }
}