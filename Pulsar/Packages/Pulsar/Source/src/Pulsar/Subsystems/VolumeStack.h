#pragma once
#include <Pulsar/Assets/VolumeSettings.h>
#include <CoreLib/Type.h>
#include <unordered_map>
#include <memory>

namespace pulsar
{
    // Holds blended VolumeSettings components, keyed by their runtime Type.
    // Similar to URP's VolumeStack, each PostProcessPass reads its own settings
    // directly from the stack via GetComponent<T>().
    class VolumeStack
    {
    public:
        template<typename T>
        T* GetComponent() const
        {
            return dynamic_cast<T*>(GetComponent(cltypeof<T>()));
        }

        VolumeSettings* GetComponent(Type* type) const
        {
            auto it = m_components.find(type);
            return it != m_components.end() ? it->second.get() : nullptr;
        }

        bool HasAnyComponent() const { return !m_components.empty(); }

        void AddComponent(Type* type, SPtr<VolumeSettings> component)
        {
            m_components[type] = component;
        }

    private:
        std::unordered_map<Type*, SPtr<VolumeSettings>> m_components;
    };

} // namespace pulsar
