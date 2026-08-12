#pragma once
#include <Pulsar/Assembly.h>
#include <Pulsar/Settings/RenderSettings.h>
#include <CoreLib/Object.h>
#include <type_traits>
#include <unordered_map>

namespace pulsar
{
    class GlobalSettings
    {
    public:
        GlobalSettings();

        RenderSettings* GetRenderSettings() { return m_renderSettings.get(); }

        Object* Register(Type* type);
        Object* Get(Type* type);

        template<typename T>
            requires std::is_base_of_v<Object, T>
        T* Get()
        {
            return static_cast<T*>(Get(cltypeof<T>()));
        }

    private:
        SPtr<RenderSettings> m_renderSettings;
        std::unordered_map<Type*, SPtr<Object>> m_registeredSettings;
    };
}