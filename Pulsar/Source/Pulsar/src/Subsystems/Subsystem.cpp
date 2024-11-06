#include "Subsystems/Subsystem.h"

namespace pulsar
{

    void Subsystem::OnInitializing()
    {
    }
    void Subsystem::OnInitialized()
    {
    }
    void Subsystem::OnTerminate()
    {
    }

    static auto& Systems()
    {
        static std::set<Type*> subsystems;
        return subsystems;
    }

    void SubsystemManager::RegisterSubsystem(Type* type)
    {
        Systems().insert(type);
    }
    void SubsystemManager::UnregisterSubsystem(Type* type)
    {
        Systems().erase(type);
    }
    const std::set<Type*>& SubsystemManager::GetAllSubsystems()
    {
        return Systems();
    }

} // namespace pulsar