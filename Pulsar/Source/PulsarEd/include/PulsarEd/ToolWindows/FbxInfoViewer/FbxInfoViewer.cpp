#include "FbxInfoViewer.h"

#include "CoreLib.Platform/Window.h"
#include "PropertyControls/PropertyControl.h"

#include <fbxsdk.h>
#include <stack>

namespace pulsared
{
    FbxInfoViewer::FbxInfoViewer()
    {
    }

    #ifdef IOS_REF
    #undef IOS_REF
    #define IOS_REF (*(pManager->GetIOSettings()))
    #endif
    static void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
    {
        // The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
        pManager = FbxManager::Create();
        if (!pManager)
        {
            FBXSDK_printf("Error: Unable to create FBX Manager!\n");
            exit(1);
        }
        else
            FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

        // Create an IOSettings object. This object holds all import/export settings.
        FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
        pManager->SetIOSettings(ios);

        // Load plugins from the executable directory (optional)
        FbxString lPath = FbxGetApplicationDirectory();
        pManager->LoadPluginsDirectory(lPath.Buffer());

        // Create an FBX scene. This object holds most objects imported/exported from/to files.
        pScene = FbxScene::Create(pManager, "My Scene");
        if (!pScene)
        {
            FBXSDK_printf("Error: Unable to create FBX scene!\n");
            exit(1);
        }
    }
    static bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
    {
        int lFileMajor, lFileMinor, lFileRevision;
        int lSDKMajor, lSDKMinor, lSDKRevision;
        // int lFileFormat = -1;
        int lAnimStackCount;
        bool lStatus;
        char lPassword[1024];

        // Get the file version number generate by the FBX SDK.
        FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

        // Create an importer.
        FbxImporter* lImporter = FbxImporter::Create(pManager, "");

        // Initialize the importer by providing a filename.
        const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
        lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

        if (!lImportStatus)
        {
            FbxString error = lImporter->GetStatus().GetErrorString();
            FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
            FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

            if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
            {
                FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
                FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
            }

            return false;
        }

        FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

        if (lImporter->IsFBX())
        {
            FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

            // From this point, it is possible to access animation stack information without
            // the expense of loading the entire file.

            FBXSDK_printf("Animation Stack Information\n");

            lAnimStackCount = lImporter->GetAnimStackCount();

            FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
            FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
            FBXSDK_printf("\n");

            for (int i = 0; i < lAnimStackCount; i++)
            {
                FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

                FBXSDK_printf("    Animation Stack %d\n", i);
                FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
                FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

                // Change the value of the import name if the animation stack should be imported
                // under a different name.
                FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

                // Set the value of the import state to false if the animation stack should be not
                // be imported.
                FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
                FBXSDK_printf("\n");
            }

            // Set the import states. By default, the import states are always set to
            // true. The code below shows how to change these states.
            IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
            IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
            IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
            IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
            IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
            IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
            IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
        }

        // Import the scene.
        lStatus = lImporter->Import(pScene);

        // Destroy the importer.
        lImporter->Destroy();

        return lStatus;
    }
    static void DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
    {
        // Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
        if (pManager)
            pManager->Destroy();
        if (pExitStatus)
            FBXSDK_printf("Program Success!\n");
    }

    static const char* FBXTypeToString(FbxNodeAttribute::EType type)
    {
        switch (type)
        {
        case FbxNodeAttribute::eUnknown: return "Unknown";
        case FbxNodeAttribute::eNull:return "Null";
        case FbxNodeAttribute::eMarker:return "Marker";
        case FbxNodeAttribute::eSkeleton:return "Skeleton";
        case FbxNodeAttribute::eMesh:return "Mesh";
        case FbxNodeAttribute::eNurbs:return "Nurbs";
        case FbxNodeAttribute::ePatch:return "Patch";
        case FbxNodeAttribute::eCamera:return "Camera" ;
        case FbxNodeAttribute::eCameraStereo:return "CameraStereo" ;
        case FbxNodeAttribute::eCameraSwitcher:return "CameraSwitcher" ;
        case FbxNodeAttribute::eLight:return "Light" ;
        case FbxNodeAttribute::eOpticalReference:return "OpticalReference" ;
        case FbxNodeAttribute::eOpticalMarker:return "OpticalMarker" ;
        case FbxNodeAttribute::eNurbsCurve:return "NurbsCurve" ;
        case FbxNodeAttribute::eTrimNurbsSurface:return "TrimNurbsSurface" ;
        case FbxNodeAttribute::eBoundary:return "Boundary" ;
        case FbxNodeAttribute::eNurbsSurface:return "NurbsSurface" ;
        case FbxNodeAttribute::eShape:return "Shape" ;
        case FbxNodeAttribute::eLODGroup:return "LODGroup" ;
        case FbxNodeAttribute::eSubDiv:return "SubDiv" ;
        case FbxNodeAttribute::eCachedEffect:return "CachedEffect" ;
        case FbxNodeAttribute::eLine:return "Line";
        }
        return {};
    }

    static void LoadFbxNodes(FbxInfoViewer::FbxInfoNodePtr infoNode, FbxNode* node)
    {
        infoNode->AssetName = node->GetName();
        infoNode->TypeName = node->GetTypeName();
        auto attrCount = node->GetNodeAttributeCount();

        for (int i = 0; i < attrCount; ++i)
        {
            auto attr = node->GetNodeAttributeByIndex(i);
            infoNode->AttributeType.push_back(FBXTypeToString(attr->GetAttributeType()));
        }

        const auto count = node->GetChildCount();
        for (int i = 0; i < count; ++i)
        {
            auto childNode = node->GetChild(i);
            auto childInfo = infoNode->NewChild();
            LoadFbxNodes(childInfo, childNode);
        }
    }
    static FbxInfoViewer::FbxInfoNodePtr LoadFbxNodes(FbxNode* node)
    {
        auto retval = mksptr(new FbxInfoViewer::FbxInfoNode);
        LoadFbxNodes(retval, node);
        return retval;
    }

    void FbxInfoViewer::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);

        ImGui::InputText("fbxpath", m_fbxpath, sizeof(m_fbxpath));
        ImGui::SameLine();
        if (ImGui::Button("..."))
        {
            std::filesystem::path fbxpath;
            auto hWnd = platform::window::GetMainWindowHandle();
            if (platform::window::OpenFileDialog(hWnd, "fbx|*.fbx;", "", &fbxpath))
            {
               StringUtil::strcpy(m_fbxpath, fbxpath.string());
            }
        }

        if (ImGui::Button("Refresh"))
        {
            using namespace fbxsdk;
            FbxManager* fbxManager;
            FbxScene* fbxScene;

            InitializeSdkObjects(fbxManager, fbxScene);
            assert(LoadScene(fbxManager, fbxScene, m_fbxpath));

            m_root = LoadFbxNodes(fbxScene->GetRootNode());

            DestroySdkObjects(fbxManager, 0);
        }


        ImGui::Columns(2);
        if (m_root)
        {
            ShowNode(m_root);
        }
        ImGui::NextColumn();
        if (m_selected)
        {
            if (PImGui::PropertyGroup("AttrType"))
            {
                if (PImGui::BeginPropertyLines())
                {
                    for (int i = 0; i < m_selected->AttributeType.size(); ++i)
                    {
                        auto& item = m_selected->AttributeType.at(i);
                        PImGui::PropertyLineText(std::to_string(i).c_str(), item.c_str());
                    }

                    PImGui::EndPropertyLines();
                }
            }

        }
        ImGui::Columns(1);
    }

    void FbxInfoViewer::ShowNode(FbxInfoNodePtr node)
    {
        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanFullWidth;

        if (node->GetChildren().empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (node == m_selected)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        bool opened = ImGui::TreeNodeEx(node->AssetName.c_str(), flags);
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            m_selected = node;
        }
        if (opened)
        {
            for (auto child : node->GetChildren())
            {
                ShowNode(child);
            }
            ImGui::TreePop();
        }
    }
} // pulsared