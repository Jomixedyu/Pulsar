#include "Editors/SceneEditor/SceneEditor.h"

#include "EditorAppInstance.h"
#include "EditorWorld.h"
#include <CoreLib.Platform/Window.h>
#include "Editors/SceneEditor/SceneEditorWindow.h"
#include "Menus/Menu.h"
#include "Menus/MenuEntrySubMenu.h"
#include "Pulsar/AssetManager.h"
#include "Pulsar/Components/BoxShape3DComponent.h"
#include "Pulsar/Components/DirectionalLightComponent.h"
#include "Pulsar/Components/PointLightComponent.h"
#include "Pulsar/Components/SkyLightComponent.h"
#include "Pulsar/Components/SphereShape3DComponent.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"
#include "Pulsar/Scene.h"
#include "ToolWindows/FbxInfoViewer/FbxInfoViewer.h"
#include "ToolWindows/MenuDebugTool.h"
#include "ToolWindows/ObjectDebugTool.h"
#include "ToolWindows/ShaderDebugTool.h"
#include "ToolWindows/WorldDebugTool.h"
#include "Editors/CommonPanel/ConsoleWindow.h"
#include "Windows/EditorWindowManager.h"
#include "Editors/CommonPanel/OutlinerWindow.h"
#include "Editors/CommonPanel/PropertiesWindow.h"
#include "Editors/CommonPanel/SceneWindow.h"
#include "Editors/CommonPanel/WorkspaceWindow.h"
#include "Workspace.h"

namespace pulsared
{
    static void InitBasicMenu(SceneEditor* editor)
    {
        auto mainMenu = MenuManager::AddMenu(editor->GetMenuName());
        {
            MenuEntrySubMenu_sp file = mksptr(new MenuEntrySubMenu("File"));
            file->Priority = 10;
            mainMenu->AddEntry(file);

            auto openWorkSpace = mksptr(new MenuEntryButton("Open Workspace"));
            openWorkSpace->Action = MenuAction::FromRaw([](SPtr<MenuContexts> ctx) {
                Workspace::OpenDialogUserWorkspace();
            });
            file->AddEntry(openWorkSpace);

            file->AddEntry(mksptr(new MenuEntrySeparate("")));

            auto newScene = mksptr(new MenuEntryButton("New Scene"));
            newScene->Action = MenuAction::FromLambda([editor](SPtr<MenuContexts> ctx) {
                editor->NewScene();
            });
            file->AddEntry(newScene);

            auto openScene = mksptr(new MenuEntryButton("Open Scene"));
            openScene->Action = MenuAction::FromLambda([editor](SPtr<MenuContexts> ctx) {
                editor->OpenScene();
            });
            file->AddEntry(openScene);

            auto saveScene = mksptr(new MenuEntryButton("Save Scene"));
            saveScene->Action = MenuAction::FromLambda([editor](SPtr<MenuContexts> ctx) {
                editor->SaveScene();
            });
            file->AddEntry(saveScene);

            auto saveSceneAs = mksptr(new MenuEntryButton("Save Scene As"));
            saveSceneAs->Action = MenuAction::FromLambda([editor](SPtr<MenuContexts> ctx) {
                editor->SaveSceneAs();
            });
            file->AddEntry(saveSceneAs);
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Edit"));
            menu->Priority = 20;
            mainMenu->AddEntry(menu);
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Node"));
            menu->Priority = 200;
            mainMenu->AddEntry(menu);
            {
                auto entry = mksptr(new MenuEntryButton("Create Node"));
                menu->AddEntry(entry);
                entry->Action = MenuAction::FromLambda([](MenuContexts_rsp) {
                    auto newNode = World::Current()->GetFocusScene()->NewNode("New Node");
                    World::Current()->GetSelection().Clear();
                    World::Current()->GetSelection().Select(newNode);
                });
            }

            auto shapeMenu =  mksptr(new MenuEntrySubMenu("Shapes"));
            menu->AddEntry(shapeMenu);
            {
                auto entry = mksptr(new MenuEntryButton("Create Sphere"));
                shapeMenu->AddEntry(entry);
                entry->Action = MenuAction::FromLambda([](MenuContexts_rsp) {
                    auto newNode = World::Current()->GetFocusScene()->NewNode("New Sphere");
                    newNode->AddComponent<SphereShape3DComponent>();

                    auto renderer = newNode->AddComponent<StaticMeshRendererComponent>();
                    renderer->SetStaticMesh(AssetManager::Get()->LoadAsset<StaticMesh>(BuiltinAsset::Shapes_Sphere));
                    renderer->SetMaterial(0, AssetManager::Get()->LoadAsset<Material>(BuiltinAsset::Material_Lambert));

                });
            }
            {
                auto entry = mksptr(new MenuEntryButton("Create Cube"));
                shapeMenu->AddEntry(entry);
                entry->Action = MenuAction::FromLambda([](MenuContexts_rsp) {
                    auto newNode = World::Current()->GetFocusScene()->NewNode("New Cube");
                    newNode->AddComponent<BoxShape3DComponent>();

                    auto renderer = newNode->AddComponent<StaticMeshRendererComponent>();
                    renderer->AddMaterial();
                    renderer->SetMaterial(0, AssetManager::Get()->LoadAsset<Material>(BuiltinAsset::Material_Lambert));
                    renderer->SetStaticMesh(AssetManager::Get()->LoadAsset<StaticMesh>(BuiltinAsset::Shapes_Cube));

                });
            }
            {
                auto entry = mksptr(new MenuEntryButton("Create Plane"));
                shapeMenu->AddEntry(entry);
                entry->Action = MenuAction::FromLambda([](MenuContexts_rsp) {
                    auto renderer = World::Current()->GetFocusScene()->NewNode("New Plane")
                        ->AddComponent<StaticMeshRendererComponent>();
                    renderer->SetStaticMesh(AssetManager::Get()->LoadAsset<StaticMesh>(BuiltinAsset::Shapes_Plane));
                    renderer->SetMaterial(0, AssetManager::Get()->LoadAsset<Material>(BuiltinAsset::Material_Lambert));

                });
            }

            auto light3dMenu = mksptr(new MenuEntrySubMenu("Light3d"));
            menu->AddEntry(light3dMenu);
            {
                auto entry = mksptr(new MenuEntryButton("Create Sky Light"));
                light3dMenu->AddEntry(entry);
                entry->Action = MenuAction::FromLambda([](MenuContexts_rsp) {
                    World::Current()->GetFocusScene()->NewNode("New Sky Light")
                        ->AddComponent<SkyLightComponent>();
                });
            }
            {
                auto entry = mksptr(new MenuEntryButton("Create Directional Light"));
                light3dMenu->AddEntry(entry);
                entry->Action = MenuAction::FromLambda([](MenuContexts_rsp) {
                    World::Current()->GetFocusScene()->NewNode("New Directional Light")
                        ->AddComponent<DirectionalLightComponent>();
                });
            }
            {
                auto entry = mksptr(new MenuEntryButton("Create Point Light"));
                light3dMenu->AddEntry(entry);
                entry->Action = MenuAction::FromLambda([](MenuContexts_rsp) {
                    World::Current()->GetFocusScene()->NewNode("New Point Light")
                        ->AddComponent<PointLightComponent>();
                });
            }
        }

        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Components"));
            menu->Priority = 500;
            mainMenu->AddEntry(menu);

            array_list<Type*> components;
            array_list<Type*> noCategoryComponents;
            for (auto type : AssemblyManager::GlobalSearchType(cltypeof<Component>()))
            {
                if (type->IsDefinedAttribute(cltypeof<AbstractComponentAttribute>(), false))
                {
                    continue;
                }
                if (type->IsDefinedAttribute(cltypeof<CategoryAttribute>(), false))
                {
                    components.push_back(type);
                }
                else
                {
                    // noCategoryComponents.push_back(type);
                }
            }

            components.append_range(noCategoryComponents);
            for (auto type : components)
            {
                auto targetMenu = menu;
                if (auto category = type->GetAttribute<CategoryAttribute>(false))
                {
                    auto categoryEntry = menu->FindSubMenuEntry(category->GetCategory());
                    if (!categoryEntry)
                    {
                        categoryEntry = mksptr(new MenuEntrySubMenu(string{category->GetCategory()}));
                        menu->AddEntry(categoryEntry);
                    }
                    targetMenu = categoryEntry;
                }

                auto itemEntry = mksptr(new MenuEntryButton(type->GetName(),
                                                            ComponentInfoManager::GetFriendlyComponentName(type)));
                targetMenu->AddEntry(itemEntry);
                itemEntry->Action = MenuAction::FromLambda([](MenuContexts_rsp ctxs) {
                    auto edworld = dynamic_cast<EditorWorld*>(EditorWorld::GetPreviewWorld());

                    if (auto node = cast<Node>(edworld->GetSelection().GetSelected()))
                    {
                        Type* type = AssemblyManager::GlobalFindType(ctxs->EntryName);
                        node->AddComponent(type);
                    }
                });
            }
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Tool"));
            menu->Priority = 800;
            mainMenu->AddEntry(menu);
            {
                auto entry = mksptr(new MenuEntryButton("MenuDebug"));
                entry->Action = MenuAction::FromRaw([](SPtr<MenuContexts> ctx) {
                    ToolWindow::OpenToolWindow<MenuDebugTool>();
                });
                menu->AddEntry(entry);
            }
            {
                auto entry = mksptr(new MenuEntryButton("ObjectDebug"));
                entry->Action = MenuAction::FromRaw([](SPtr<MenuContexts> ctx) {
                    ToolWindow::OpenToolWindow<ObjectDebugTool>();
                });
                menu->AddEntry(entry);
            }
            {
                auto entry = mksptr(new MenuEntryButton("WorldDebug"));
                entry->Action = MenuAction::FromRaw([](SPtr<MenuContexts> ctx) {
                    ToolWindow::OpenToolWindow<WorldDebugTool>();
                });
                menu->AddEntry(entry);
            }
            {
                auto entry = mksptr(new MenuEntryButton("ShaderDebugTool"));
                entry->Action = MenuAction::FromRaw([](SPtr<MenuContexts> ctx) {
                    ToolWindow::OpenToolWindow<ShaderDebugTool>();
                });
                menu->AddEntry(entry);
            }
            {
                auto entry = mksptr(new MenuEntryButton("FbxInfoViewer"));
                entry->Action = MenuAction::FromRaw([](SPtr<MenuContexts> ctx) {
                    ToolWindow::OpenToolWindow<FbxInfoViewer>();
                });
                menu->AddEntry(entry);
            }
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Build"));
            menu->Priority = 900;
            mainMenu->AddEntry(menu);
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Window"));
            menu->Priority = 1000;
            mainMenu->AddEntry(menu);
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Help"));
            menu->Priority = 2000;
            mainMenu->AddEntry(menu);
        }


    }

    void SceneEditor::Initialize()
    {
        base::Initialize();

        InitBasicMenu(this);
        RegisterPanelType(cltypeof<SceneWindow>());
        RegisterPanelType(cltypeof<PropertiesWindow>());
        RegisterPanelType(cltypeof<ConsoleWindow>());
        RegisterPanelType(cltypeof<WorkspaceWindow>());
        RegisterPanelType(cltypeof<OutlinerWindow>());

    }

    void SceneEditor::Terminate()
    {
        base::Terminate();
    }

    SPtr<EditorWindow> SceneEditor::OnCreateEditorWindow()
    {
        return mksptr(new SceneEditorWindow);
    }

    static string PhysicsPathToAssetPath(const std::filesystem::path& physicsPath)
    {
        auto absPath = std::filesystem::absolute(physicsPath).generic_string();

        for (auto& package : AssetDatabase::GetPackageInfos())
        {
            auto packageAssetsPath = std::filesystem::absolute(package.Path / "Assets").generic_string();
            if (absPath.size() <= packageAssetsPath.size())
                continue;

            if (absPath.substr(0, packageAssetsPath.size()) != packageAssetsPath)
                continue;

            auto relPath = absPath.substr(packageAssetsPath.size());
            if (!relPath.empty() && (relPath[0] == '/' || relPath[0] == '\\'))
                relPath = relPath.substr(1);

            auto dotPos = relPath.rfind('.');
            if (dotPos != string::npos)
                relPath = relPath.substr(0, dotPos);

            return package.Name + "/" + relPath;
        }
        return {};
    }

    void SceneEditor::NewScene()
    {
        auto world = GetEdApp()->GetEditorWorld();

        if (auto oldScene = world->GetFocusScene())
        {
            world->UnloadScene(oldScene);
        }

        auto scene = Scene::StaticCreate("NewScene");
        scene->SetObjectFlags(scene->GetObjectFlags() & ~OF_Transient);
        world->LoadScene(scene);
        world->SetFocusScene(scene);

        GetEdApp()->SetupDefaultResidentScene();
    }

    void SceneEditor::OpenScene()
    {
        std::filesystem::path selectedPath;
        if (!platform::window::OpenFileDialog(platform::window::GetMainWindowHandle(),
            "Scene(*.pa)|*.pa;", "", &selectedPath))
        {
            return;
        }

        auto assetPath = PhysicsPathToAssetPath(selectedPath);
        if (assetPath.empty())
        {
            Logger::Log("Selected path is not inside a package Assets folder.", LogLevel::Error);
            return;
        }

        auto asset = AssetDatabase::LoadAssetAtPath(assetPath);
        if (!asset)
        {
            Logger::Log("Failed to load scene asset.", LogLevel::Error);
            return;
        }

        auto scene = cast<Scene>(asset);
        if (!scene)
        {
            Logger::Log("Selected asset is not a Scene.", LogLevel::Error);
            return;
        }

        auto world = GetEdApp()->GetEditorWorld();
        if (auto oldScene = world->GetFocusScene())
        {
            world->UnloadScene(oldScene);
        }

        scene->SetObjectFlags(scene->GetObjectFlags() & ~OF_Transient);
        world->LoadScene(scene);
        world->SetFocusScene(scene);
    }

    void SceneEditor::SaveScene()
    {
        auto focusScene = GetEdApp()->GetEditorWorld()->GetFocusScene();
        if (!focusScene)
        {
            Logger::Log("No active scene to save.", LogLevel::Warning);
            return;
        }

        auto path = AssetDatabase::GetPathByAsset(focusScene);
        if (path.empty())
        {
            SaveSceneAs();
        }
        else
        {
            AssetDatabase::MarkDirty(focusScene);
            AssetDatabase::Save(focusScene);
        }
    }

    bool SceneEditor::SaveSceneAs()
    {
        auto focusScene = GetEdApp()->GetEditorWorld()->GetFocusScene();
        if (!focusScene)
        {
            Logger::Log("No active scene to save.", LogLevel::Warning);
            return false;
        }

        std::filesystem::path selectedPath;
        if (!platform::window::SaveFileDialog(platform::window::GetMainWindowHandle(),
            "Scene(*.pa)|*.pa;", "", &selectedPath))
        {
            return false;
        }

        auto assetPath = PhysicsPathToAssetPath(selectedPath);
        if (assetPath.empty())
        {
            Logger::Log("Selected path is not inside a package Assets folder.", LogLevel::Error);
            return false;
        }

        auto oldPath = AssetDatabase::GetPathByAsset(focusScene);
        if (!oldPath.empty())
        {
            if (oldPath != assetPath)
            {
                AssetDatabase::Rename(oldPath, assetPath);
            }
        }
        else
        {
            if (!AssetDatabase::CreateAsset(focusScene, assetPath))
            {
                Logger::Log("Failed to create asset at path: " + assetPath, LogLevel::Warning);
                return false;
            }
        }

        AssetDatabase::MarkDirty(focusScene);
        AssetDatabase::Save(focusScene);
        return true;
    }
} // namespace pulsared