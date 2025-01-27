#include "FbxInfoViewer.h"

#include "CoreLib.Platform/Window.h"
#include "PropertyControls/PropertyControl.h"

#include <fbxsdk.h>
#include <stack>


namespace fbxinfo
{
    #ifdef IOS_REF
    #undef IOS_REF
    #define IOS_REF (*(pManager->GetIOSettings()))
    #endif
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


    // static void LoadFbxNodes(FbxInfoViewer::FbxInfoNodePtr infoNode, FbxNode* node)
    // {
    //     infoNode->AssetName = node->GetName();
    //     infoNode->TypeName = node->GetTypeName();
    //     auto attrCount = node->GetNodeAttributeCount();
    //
    //     for (int i = 0; i < attrCount; ++i)
    //     {
    //         auto attr = node->GetNodeAttributeByIndex(i);
    //         infoNode->AttributeType.push_back(FBXTypeToString(attr->GetAttributeType()));
    //     }
    //
    //     const auto count = node->GetChildCount();
    //     for (int i = 0; i < count; ++i)
    //     {
    //         auto childNode = node->GetChild(i);
    //         auto childInfo = infoNode->NewChild();
    //         LoadFbxNodes(childInfo, childNode);
    //     }
    // }
    // static FbxInfoNodePtr LoadFbxNodes(FbxNode* node)
    // {
    //     auto retval = mksptr(new FbxInfoViewer::FbxInfoNode);
    //     LoadFbxNodes(retval, node);
    //     return retval;
    // }

    struct FbxInfoViewerContext
    {
        char FbxPath[255]{0};
        bool Opened = false;
        FbxManager* FbxManager = nullptr;
        FbxScene* FbxScene = nullptr;
    };

    inline const char* to_string(FbxAxisSystem::ECoordSystem system)
    {
        switch (system)
        {
        case FbxAxisSystem::eRightHanded: return "RightHanded";
        case FbxAxisSystem::eLeftHanded: return "LeftHanded";
        }
        return nullptr;
    }
    inline const char* to_string(FbxAxisSystem::EFrontVector e)
    {
        switch (e)
        {
        case FbxAxisSystem::eParityEven: return "ParityEven";
        case FbxAxisSystem::eParityOdd: return "ParityOdd";
        }
        return nullptr;
    }
    inline const char* to_string(FbxAxisSystem::EUpVector e)
    {
        switch (e)
        {
        case FbxAxisSystem::eXAxis: return "XAxis";
        case FbxAxisSystem::eYAxis: return "YAxis";
        case FbxAxisSystem::eZAxis: return "ZAxis";
        }
        return nullptr;
    }
    enum FvxDataTableColumn
    {
        eName,
        eType,
        ePosX,
        ePosY,
        ePosZ,
        eRotX,
        eRotY,
        eRotZ,
        eScaleX,
        eScaleY,
        eScaleZ,
        eMAX_NUM
    };
    void DrawFbxTree(FbxInfoViewerContext* ctx, FbxNode* node)
    {
        ImGuiTreeNodeFlags base_flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanFullWidth;

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(eName);
        bool isOpened = ImGui::TreeNodeEx(node, base_flags, node->GetName());

        auto pos = node->LclTranslation.Get();
        auto rot = node->LclRotation.Get();
        auto scale = node->LclScaling.Get();

        ImGui::TableSetColumnIndex(eType);
        ImGui::Text(node->GetTypeName());


        ImGui::TableSetColumnIndex(ePosX);
        ImGui::Text("%f", pos[0]);;
        ImGui::TableSetColumnIndex(ePosY);
        ImGui::Text("%f", pos[1]);
        ImGui::TableSetColumnIndex(ePosZ);
        ImGui::Text("%f", pos[2]);

        ImGui::TableSetColumnIndex(eRotX);
        ImGui::Text("%f", rot[0]);
        ImGui::TableSetColumnIndex(eRotY);
        ImGui::Text("%f", rot[1]);
        ImGui::TableSetColumnIndex(eRotZ);
        ImGui::Text("%f", rot[2]);

        ImGui::TableSetColumnIndex(eScaleX);
        ImGui::Text("%f", scale[0]);
        ImGui::TableSetColumnIndex(eScaleY);
        ImGui::Text("%f", scale[1]);
        ImGui::TableSetColumnIndex(eScaleZ);
        ImGui::Text("%f", scale[2]);

        if (isOpened)
        {
            for (int i = 0; i < node->GetChildCount(); ++i)
            {
                auto child = node->GetChild(i);
                DrawFbxTree(ctx, child);
            }

            ImGui::TreePop();
        }
    }

    void DrawUI(FbxInfoViewerContext* ctx)
    {
        if (!ctx) return;
        if (!ctx->Opened)
        {
            ImGui::InputText("fbx path", ctx->FbxPath, sizeof(ctx->FbxPath));
            ImGui::SameLine();
            if (ImGui::Button("Open"))
            {
                using namespace fbxsdk;

                InitializeSdkObjects(ctx->FbxManager, ctx->FbxScene);
                assert(LoadScene(ctx->FbxManager, ctx->FbxScene, ctx->FbxPath));
                ctx->Opened = true;
            }
        }
        else
        {
            if (ImGui::Button("Close"))
            {
                DestroySdkObjects(ctx->FbxManager, false);
            }
        }

        if (ctx->Opened)
        {
            static const char* fbxviewId = "fbxviewId";
            ImGui::Columns(2, fbxviewId);

            if (ImGui::BeginChild("scene_data"))
            {
                bool b = ImGui::BeginTable("_data", FvxDataTableColumn::eMAX_NUM, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable);
                if (b)
                {
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("P.x");
                    ImGui::TableSetupColumn("P.y");
                    ImGui::TableSetupColumn("P.z");
                    ImGui::TableSetupColumn("R.x");
                    ImGui::TableSetupColumn("R.y");
                    ImGui::TableSetupColumn("R.z");
                    ImGui::TableSetupColumn("S.x");
                    ImGui::TableSetupColumn("S.y");
                    ImGui::TableSetupColumn("S.z");
                    ImGui::TableHeadersRow();

                    DrawFbxTree(ctx, ctx->FbxScene->GetRootNode());
                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();


            ImGui::NextColumn();

            auto& settings = ctx->FbxScene->GetGlobalSettings();

            if (ImGui::BeginChild("detail"))
            {

                if (pulsared::PImGui::PropertyGroup("System"))
                {
                    if (pulsared::PImGui::BeginPropertyLines())
                    {
                        auto axisSystem = settings.GetAxisSystem();
                        pulsared::PImGui::PropertyLineText("CoordSystem", to_string(axisSystem.GetCoorSystem()));
                        int sign = 1;
                        pulsared::PImGui::PropertyLineText("Up", to_string(axisSystem.GetUpVector(sign)));
                        pulsared::PImGui::PropertyLineText("Front", to_string(axisSystem.GetFrontVector(sign)));
                        ;
                        pulsared::PImGui::EndPropertyLines();
                    }
                }
                if (pulsared::PImGui::PropertyGroup("Unit"))
                {
                    if (pulsared::PImGui::BeginPropertyLines())
                    {
                        auto axisSystem = settings.GetAxisSystem();

                        pulsared::PImGui::PropertyLineText("ScaleFactor", std::to_string(settings.GetSystemUnit().GetScaleFactor()));
                        pulsared::PImGui::PropertyLineText("Multiplier", std::to_string(settings.GetSystemUnit().GetMultiplier()));
                        pulsared::PImGui::PropertyLineText("OriginalScaleFactor", std::to_string(settings.GetOriginalSystemUnit().GetScaleFactor()));
                        pulsared::PImGui::PropertyLineText("OriginalMultiplier", std::to_string(settings.GetOriginalSystemUnit().GetMultiplier()));
                        ;
                        pulsared::PImGui::EndPropertyLines();
                    }
                }
            }
            ImGui::EndChild();
            ImGui::Columns(1, fbxviewId);
        }
    }
}

namespace pulsared
{
    FbxInfoViewer::FbxInfoViewer()
    {
    }



    void FbxInfoViewer::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);
        fbxinfo::DrawUI((fbxinfo::FbxInfoViewerContext*)m_fbxctx);
    }

    void FbxInfoViewer::OnOpen()
    {
        m_fbxctx = new fbxinfo::FbxInfoViewerContext{};
    }
    void FbxInfoViewer::OnClose()
    {
        delete (fbxinfo::FbxInfoViewerContext*)m_fbxctx;
    }
} // pulsared