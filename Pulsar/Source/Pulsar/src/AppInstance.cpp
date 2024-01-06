#include "AppInstance.h"

namespace pulsar
{
    array_list<gfx::GFXApi> AppInstance::GetSupportedApis()
    {
        return {gfx::GFXApi::Vulkan};
    }
    Subsystem* AppInstance::GetSubsystemByType(Type* type)
    {
        for (auto& subsystem : this->subsystems)
        {
            if (subsystem->GetType() == type)
            {
                return subsystem.get();
            }
        }
        return nullptr;
    }

    array_list<Subsystem*> AppInstance::GetSubsystemsByType(Type* type, bool include)
    {
        array_list<Subsystem*> ret;
        for (auto& subsystem : this->subsystems)
        {
            if (include)
            {
                if (type->IsInstanceOfType(subsystem.get()))
                {
                    ret.push_back(subsystem.get());
                }
            }
            else
            {
                if (subsystem->GetType() == type)
                {
                    ret.push_back(subsystem.get());
                }
            }
        }
        return ret;
    }
}