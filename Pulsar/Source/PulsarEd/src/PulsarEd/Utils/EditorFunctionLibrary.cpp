#include "EditorFunctionLibrary.h"

#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Util/TextureSaveUtil.h>
#include "EditorWorld.h"

namespace pulsared
{
    std::string EditorFunctionLibrary::CaptureScreenshot()
    {
        auto world = dynamic_cast<EditorWorld*>(EditorWorld::GetPreviewWorld());
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
