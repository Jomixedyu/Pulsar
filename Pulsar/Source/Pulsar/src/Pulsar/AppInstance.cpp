#include "AppInstance.h"

namespace pulsar
{
    array_list<gfx::GFXApi> AppInstance::GetSupportedApis()
    {
        return {gfx::GFXApi::Vulkan};
    }

}