#include "AppInstance.h"

namespace pulsar
{
    AppInstance::AppInstance() = default;
    AppInstance::~AppInstance() = default;

    array_list<gfx::GFXApi> AppInstance::GetSupportedApis()
    {
        return {gfx::GFXApi::Vulkan};
    }
}