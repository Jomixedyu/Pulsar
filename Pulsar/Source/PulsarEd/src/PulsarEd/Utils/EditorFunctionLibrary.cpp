#include "EditorFunctionLibrary.h"

#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Util/TextureSaveUtil.h>
#include "EditorWorld.h"
#include "Editors/SceneEditor/SceneEditor.h"

namespace pulsared
{
    std::string EditorFunctionLibrary::CaptureScreenshot()
    {
        auto sceneEditor = SceneEditor::GetCurrent();
        auto world = sceneEditor ? dynamic_cast<EditorWorld*>(sceneEditor->GetPreviewWorld()) : nullptr;
        if (!world)
        {
            return "";
        }

        auto camera = world->GetCurrentCamera();
        if (!camera)
        {
            return "";
        }

        return pulsar::TextureSaveUtil::CaptureCameraScreenshot(camera.GetPtr());
    }
}
