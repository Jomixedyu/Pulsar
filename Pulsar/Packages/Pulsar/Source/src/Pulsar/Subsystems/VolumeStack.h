#pragma once
#include <Pulsar/Assets/VolumeSettings.h>
#include <CoreLib/Type.h>
#include <unordered_map>
#include <memory>

namespace pulsar
{
    // Holds blended render-thread snapshots, keyed by their source settings type.
    // Similar to URP's VolumeStack, each post-process render feature reads its own settings
    // directly from the stack via GetComponent<T>().
    class VolumeStack
    {
    public:
        template<typename T>
        T* GetComponent() const
        {
            return dynamic_cast<T*>(GetComponent(cltypeof<typename T::SettingsType>()));
        }

        VolumeRenderSnapshot* GetComponent(Type* type) const
        {
            auto it = m_components.find(type);
            return it != m_components.end() ? it->second.get() : nullptr;
        }

        bool HasAnyComponent() const { return !m_components.empty(); }

        void AddComponent(SPtr<VolumeRenderSnapshot> component)
        {
            if (component)
                m_components[component->GetSettingsType()] = component;
        }

    private:
        std::unordered_map<Type*, SPtr<VolumeRenderSnapshot>> m_components;
    };

} // namespace pulsar
